%% ==================== LOOCV TRAINING & TESTING ==================== %%
clear;

% ---------- Paths ----------
inpPath = 'Y:\Arnav\MT\Data\input\processed';
outPath = 'Y:\Arnav\MT\Data\output\processed';
files = dir(fullfile(inpPath, '*.mat'));
numFiles = numel(files);

targetIDs = [1, 7];            % U92, PU94
targetNames = ["U92", "PU94"];

% ---------- Precompute Features & Labels ----------
% [Keep your existing feature extraction loop here - it is mathematically sound]
% Ensure X_all (N x 18) and Y_all (N x 2) are populated as per your snippet.
X_all = zeros(numFiles, 18);
Y_all = false(numFiles, numel(targetIDs));

for i = 1:numFiles
    dataI = load(fullfile(inpPath, files(i).name));
    dataO = load(fullfile(outPath, files(i).name));

    fI = fields(dataI); volInp = double(dataI.(fI{1}));
    fO = fields(dataO); volOut = dataO.(fO{1});

    volInp(volInp > 1) = 1; volInp(volInp < 0) = 0;
    rawValues = volInp(:);

    % Adaptive threshold
    thresh = graythresh(rawValues);

    % ---------- INTENSITY FEATURES ----------
    X_all(i,1) = prctile(rawValues, 99);
    X_all(i,2) = max(rawValues);
    X_all(i,3) = mean(rawValues(rawValues >= prctile(rawValues,90)));
    X_all(i,4) = std(rawValues);
    X_all(i,5) = log(sum(rawValues.^2)+eps);
    X_all(i,6) = sum(rawValues>thresh)/numel(rawValues);

    % Entropy
    p = rawValues / (sum(rawValues)+eps);
    X_all(i,7) = -sum(p.*log2(p+eps));

    % GLCM (2D max projection)
    img2D = max(volInp,[],3); imgNorm = rescale(img2D);
    glcm = graycomatrix(imgNorm, 'NumLevels',16);
    stats = graycoprops(glcm);
    X_all(i,8)  = stats.Contrast;
    X_all(i,9)  = stats.Correlation;
    X_all(i,10) = stats.Energy;
    X_all(i,11) = stats.Homogeneity;

    % Connected components
    bw = volInp>thresh; bw = bwareaopen(bw,30);
    cc = bwconncomp(bw,26);
    if cc.NumObjects>0
        sizes = cellfun(@numel, cc.PixelIdxList);
        X_all(i,12) = max(sizes);
        X_all(i,13) = mean(sizes);
        X_all(i,14) = numel(sizes);
    else
        X_all(i,12:14) = 0;
    end

    % Spatial spread
    [x,y,z] = ind2sub(size(volInp), find(bw));
    if ~isempty(x)
        X_all(i,15) = std(x); X_all(i,16) = std(y); X_all(i,17) = std(z);
    else
        X_all(i,15:17) = 0;
    end

    % Skewness
    X_all(i,18) = skewness(rawValues);

    % Labels
    uniqueIDs = unique(volOut(:));
    for j = 1:numel(targetIDs)
        Y_all(i,j) = any(uniqueIDs == targetIDs(j));
    end
end

% ---------- LOOCV ----------
allPreds = false(numFiles, numel(targetIDs));
allScores = zeros(numFiles, numel(targetIDs));

for i = 1:numFiles
    % fprintf('LOOCV: Test volume %d / %d\n', i, numFiles);
    
    trainIdx = setdiff(1:numFiles, i);
    testIdx = i;
    
    X_train = X_all(trainIdx,:);
    Y_train = Y_all(trainIdx,:);
    X_test  = X_all(testIdx,:);

    % --- Z-Score Normalization ---
    mu = mean(X_train);
    sigma = std(X_train) + eps;
    X_train_norm = (X_train - mu) ./ sigma;
    X_test_norm  = (X_test - mu) ./ sigma;

    for j = 1:numel(targetIDs)
        currentY_train = Y_train(:,j);
        
        % Check if the training set has both classes (0 and 1)
        if numel(unique(currentY_train)) < 2
            % If only one class exists, predict that class by default
            allPreds(i,j) = currentY_train(1);
            allScores(i,j) = double(currentY_train(1));
            continue;
        end

        if j == 1 % U92
            t = templateTree('MaxNumSplits', 3, 'MinLeafSize', 5);
            model{j,1} = fitcensemble(X_train_norm, currentY_train, ...
                'Method', 'RUSBoost', 'NumLearningCycles', 50, ...
                'Learners', t, 'LearnRate', 0.1);
        else % PU94
            t = templateTree('MaxNumSplits', 2, 'MinLeafSize', 3);
            model{j,1} = fitcensemble(X_train_norm, currentY_train, ...
                'Method', 'RUSBoost', 'NumLearningCycles', 60, ...
                'Learners', t, 'LearnRate', 0.1);
        end

        [pred, scoreMat] = predict(model{j,1}, X_test_norm);
        %
        % Store results
        allPreds(i,j) = pred;
        % Handle score matrix dimensions
        if size(scoreMat,2) == 2
            allScores(i,j) = scoreMat(2);
        else
            allScores(i,j) = double(pred);
        end
    end
end


% ---------- Performance ----------
fprintf('\n================ LOOCV PERFORMANCE ================\n');
% 1. Convert logical matrices to class indices for the confusion matrix
% This assumes each row has exactly one "true" class
[~, trueLabels] = max(Y_all, [], 2);
[~, predLabels] = max(allPreds, [], 2);

for j = 1:numel(targetIDs)
    tp = sum(allPreds(:,j) & Y_all(:,j));
    fn = sum(~allPreds(:,j) & Y_all(:,j));
    tn = sum(~allPreds(:,j) & ~Y_all(:,j));
    fp = sum(allPreds(:,j) & ~Y_all(:,j));
   

    sensitivity = tp/(tp+fn+eps);
    specificity = tn/(tn+fp+eps);
    precision   = tp/(tp+fp+eps);
    f1 = 2*(precision*sensitivity)/(precision+sensitivity+eps);
    acc = (sensitivity+specificity)/2*100;

    fprintf('Material: %-5s | B.Acc: %6.2f%% | F1: %.2f | TP: %d | FN: %d\n', ...
        targetNames(j), acc, f1, tp, fn);
end

perfectMatch = all(allPreds == Y_all,2);
fprintf('Perfect Volume Match: %.2f%%\n', mean(perfectMatch)*100);
fprintf('==================================================\n');

%% Generate the Visual Confusion Matrix
% ---------- Multi-Label to Multi-Class Mapping ----------
% Create a unique ID for each combination: 
% None=0, U=1, Pu=2, Both=3
trueCombos = Y_all(:,1) + 2*Y_all(:,2); 
predCombos = allPreds(:,1) + 2*allPreds(:,2);

comboNames = {'None', 'U92', 'PU94', 'Both (U+PU)'};

% Convert to categorical for plotting
trueCats = categorical(trueCombos, 0:3, comboNames);
predCats = categorical(predCombos, 0:3, comboNames);

% ---------- Generate the Visual Confusion Matrix ----------
% We increase the figure size slightly but tighten the chart
fig = figure('Color', 'w', 'Position', [100, 100, 800, 500]); 

cm = confusionchart(trueCats, predCats);

% --- TEXT SIZE & CLARITY ---
cm.FontSize = 14;            % Increases the size of all labels and cell values
% cm.Title = 'Muon Tomography: Multi-Material Detection';

% --- SUMMARIES ---
cm.ColumnSummary = 'column-normalized'; 
cm.RowSummary = 'row-normalized';    
cm.Normalization = 'row-normalized'; 

% --- LABELS ---
xlabel(cm, 'Predicted Material');
ylabel(cm, 'True Material');

% --- REFINEMENT ---
sortClasses(cm, comboNames); 

% --- OPTIONAL: MODULAR FONT CONTROL ---
% If you want the title or labels even bigger than the cell numbers:
set(gca, 'FontSize', 15); % This affects the container if needed