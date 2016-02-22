# # Clear workspace
# rm(list = ls())
# 
# # Set working directory
# setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment3")
# 
# # Source common utilities
# source("utils.R")
# load_mnist()

# Calculate the length of the Wts vector, This function is required for neural
# network algorithm
num_wts = function(p, M, K) {
    # returns the length of the Wts vector
    #
    # p = ncol(X) # number of predictor variables
    # M = size    # number of hidden units
    # K = 1       # number of classes
    return ((p + 1) * M + K * (M + 1))
}

# A display function used for visualization of Clustering algos
printsims <- function(x, y) {
    for (i in seq(1, 10, by = 1)) {
        
        citems1 <- which(x == i)
        
        jsims <- numeric(10)
        for (j in seq(1, 10, by = 1)) {
            citems2 <- which(y == j)
            nintx <- length(interaction(citems1, citems2))
            nunion <- length(union(citems1, citems2))
            jsims[j] <- 100 * nintx / nunion        
        }
        
        jmax <- which(jsims == max(jsims))
        cat(sprintf("idx1 = %d, idx2 = %d, jsim = %f\n", i, jmax, max(jsims)))    
    }    
}

# A Wrapper function for running Neuralnet with different X matrices
run_nnet <- function(Xtrain_, Xtest_, ytrain_, ytest_){
    
    p <- ncol(Xtrain_)      # Number of features
    K <- 10                 # Number of output classes
    best_M <- 50            # Number of hidden units
    best_maxit <- 250       # Number of iterations
    best_decay <- 0.1       # Decay parameter
    
    tstart <- proc.time()
    
    nnmodel <- nnet(x=Xtrain_, y = class.ind(ytrain_),
                    softmax=TRUE,
                    size=best_M, decay=best_decay, maxit=best_maxit,
                    MaxNWts=num_wts(p, best_M, K),
                    Wts=runif(num_wts(p, best_M, K), -0.5, 0.5),
                    trace=FALSE)
    
    tend <- proc.time()
    
    elapsed <- tend[3] - tstart[3]
    ytrain.predicted <- as.numeric(predict(nnmodel, 
                                           newdata=Xtrain_, 
                                           type="class"))
    train.error <- 100 * sum(ytrain.predicted != ytrain_)/length(ytrain_)
    
    ytest.pred <- as.numeric(predict(nnmodel, 
                                     newdata=Xtest_, 
                                     type="class"))
    test.error <- 100 * sum(ytest.pred != ytest_)/length(ytest_)
    
    cat(sprintf("num_features = %d, training error %f, test error = %f, run time = %f\n", 
                ncol(Xtrain_),
                train.error, 
                test.error, 
                elapsed))    
}

# Function for dimensionality reduction using KMeans. Takes raw training/test
# data and returns distances to centers
distances_km <- function(X_, centers_) {
    nr <- nrow(X_)
    nc <- ncol(centers_)
    cat(sprintf("nr = %d, nc = %s\n", nr, nc))
    X_km <- matrix(numeric(nr * nc), nrow = nr, ncol = nc)
    for (i in seq(1,10)) {
        Cx <- centers_[i, ]
        Sx <- sweep(X_, 2, Cx, "-")
        Dx <- apply(Sx, 1, function(r){ sqrt(sum(r^2))})
        Dx
        X_km[, i] <- Dx
    }
    return(X_km)
}

# Load libraries
library(stats)
library(mclust)
library(fastICA)
library(fBasics)
library(nnet)
library(ica)

# Create training and test sets
ntrain <- 500
ntest <- 10000

# Normalize training and testing data
normFactor <- max(rtrain$x)
nxtrain <- rtrain$x[1:ntrain, ]/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- rtest$x[1:ntest, ]/normFactor - meanFactor
ytrain <- rtrain$y[1:ntrain]
ytest <- rtest$y[1:ntest]

# Full training and test sets
Xtrain_full <- rtrain$x
Xtest_full <- rtest$x
ytrain_full <- rtrain$y
ytest_full <- rtest$y

cat(sprintf("Training set has %d rows and %d columns\n", nrow(nxtrain), ncol(nxtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(nxtest), ncol(nxtest)))

#######################################################
# KMeans without dimentionality reduction
#######################################################
kmeans.clusters <- kmeans(nxtrain, 20)

# Plot clustered digits
png(file="kmeans.png")
par(mfrow=c(4,5), xaxt='n', yaxt='n')
par(cex=0.6)
par(mar=c(3, 3, 0, 0), oma=c(1, 1, 1, 1))
for (i in 1:20) {
    show_digit(kmeans.clusters$center[i,])
}
dev.off()

# Compute training error from KMeans on the full dataset
# cdigits <- c(3,8,5,7,9,1,6,2,0,6)
cdigits <- c(9,1,4,2,1,7,6,7,6,3,8,2,9,7,0,0,5,1,8,3)
idx <- kmeans.clusters$cluster
ytrain.predicted <- cdigits[idx]
training.error <- 100.0 * sum(ytrain.predicted != ytrain)/ntrain
cat(sprintf("Training error = %f\n", training.error))

# Compute error for each digit
errs = numeric(10)
for (i in 0:9) {
    errs[i+1] <- 100.0 * sum((ytrain == i) & (ytrain.predicted != i))/ntrain
}
names(errs) <- c(0:9)

png(file="kmeans_errs.png")
par(mfrow=c(1,1))
barplot(sort(errs), ylim=c(0,10))
dev.off()

###############################################
# Expectation Maximization
###############################################
ntrain_em <- 700
em.clusters <- Mclust(rtrain$x[1:ntrain_em,], 10)

# plot
png(file="exmax.png")
par(mfrow=c(4,5), xaxt='n', yaxt='n')
par(cex=0.6)
par(mar=c(3, 3, 0, 0), oma=c(1, 1, 1, 1))
for (i in 1:20) {
    show_digit(em.clusters$parameters$mean[,i])
}
dev.off()

# Training error
# cdigits <- c(8,0,6,1,9, 2,1,3,4,3, 5,6,7,8,9,4,7,0,0,2)
cdigits <- c(3,0,1,9,1, 3,5,6,7,2)
idx <- em.clusters$classification
ytrain.predicted <- cdigits[idx]
ytrain.labels <- rtrain$y[1:ntrain_em]
training.error <- 100.0 * sum(ytrain.predicted != ytrain.labels)/ntrain_em
# 53.1667 %
cat(sprintf("Training error = %f\n", training.error))

# On which digits is most error made
errs = numeric(10)
for (i in 0:9) {
    errs[i+1] <- 100.0 * sum((ytrain.labels == i) & (ytrain.predicted != i))/ntrain_em
}
names(errs) <- c(0:9)

png(file="exmax_errs.png")
par(mfrow=c(1,1))
barplot(sort(errs), ylim=c(0,10))
dev.off()

###############################################
# PCA
###############################################
pcamod <- prcomp(nxtrain, 
              scale. = FALSE, 
              center = TRUE,
              retx = TRUE,
              tol = 0.)

pcamod.var <- pcamod$sdev^2
pctvar <- cumsum(pcamod.var)/sum(pcamod.var) * 100
thresh <- 95
idx95 <- which(abs(pctvar - thresh) == min(abs(pctvar - thresh)))
png(file="pcacoverage.png")
par(mar=c(5, 5, 1, 1))
plot(pctvar, xlab = "principal components", ylab = "Percent variance covered")
lines(c(idx95, idx95), c(0, thresh), col='red', lwd=2, lty=2)
lines(c(-200, idx95), c(thresh, thresh), col='red', lwd=2, lty=2)
text(idx95 + 60, 20, sprintf("n = %d", idx95) )
dev.off()

# Plot PCA eigenvectors
png(file="pcaeigs.png")
par(mfrow=c(4,5), xaxt='n', yaxt='n')
par(cex=0.6)
par(mar=c(3, 3, 0, 0), oma=c(1, 1, 1, 1))
eigno = c(1:15, 20, 30, 40, 50, 100 )
for (i in 1:20) {
    show_digit(pcamod$rotation[, i])
}
dev.off()

# Reconstruct using truncated Eigenvalues
pc.use <- 25
rot <- pcamod$rotation[, 1:pc.use]
Xtrunc <- pcamod$x[, 1:pc.use]
Xr <- Xtrunc %*% t(rot)
Xr <- scale(Xr, center = -1 * pcamod$center)

# plot reconstructed digits
png(file="pcareconstruct.png")
par(mfrow=c(4,5), xaxt='n', yaxt='n')
par(cex=0.6)
par(mar=c(3, 3, 0, 0), oma=c(1, 1, 1, 1))
eigno = c(1:15, 20, 30, 40, 50, 100 )
for (i in 1:20) {
    show_digit(Xr[i, ])
}
dev.off()


###################################
# ICA on data
###################################
icamod <- fastICA(nxtrain, n.comp = 10)

###################################
# KMeans Clustering after PCA
###################################
cat(sprintf("\nKMeans + PCA\n"))

kmpca <- kmeans(pcamod$x, 10)

# Did we get the same clusters
printsims(kmeans.clusters$cluster, kmpca$cluster)

####################################
# EM before and after PCA
####################################
cat(sprintf("\nExpectation maximization + PCA\n"))

# Number of training samples for EM
ntrain_em <- 700

# EM Before PCA
em.clusters <- Mclust(nxtrain[1:ntrain_em, ], 10)

# EM after PCA
empca <- Mclust(pcamod$x[1:ntrain_em, ], 10)

# Did we get the same clusters
printsims(em.clusters$classification, empca$classification)

#####################################
# ICA
#####################################
icamod <- icafast(nxtrain, 100, fun="kur", maxit=1000)
png(file="icadist.png")
par(mar=c(5,5,1,1))
plot(sort(colKurtosis(icamod$S), decreasing = TRUE),
     xlab="Independent component",
     ylab="Kurtosis")
dev.off()

#####################################
# KMeans after ICA
#####################################
cat(sprintf("\nKmeans + ICA\n"))
icamod <- fastICA(nxtrain, 10)
kmica <- kmeans(icamod$S, 10)

# Did we get the same clusters
printsims(kmeans.clusters$cluster, kmica$cluster)

#####################################
# EM after ICA
#####################################
# Number of training samples for EM
ntrain_em <- 700

emica <- Mclust(icamod$S[1:ntrain_em, ], 10)

# Did we get the same clusters
printsims(em.clusters$classification, emica$classification)

####################################
# Neuralnet without 
####################################
# run_nnet(nxtrain, nxtest, ytrain, ytest)

#######################################################
# Neuralnet + PCA, loop through number of PCA components 
#######################################################
tols <- c(0.1, 0.2, 0.35, 0.5)
for (i in seq(1, 10, by = 1)) {
    pcamod <- prcomp(nxtrain, 
                     scale. = FALSE, 
                     center = TRUE,
                     retx = TRUE,
                     tol = tols[i])
    
    Xtrain_pca <- pcamod$x
    Xtest_pca <- predict(pcamod, nxtest)
    run_nnet(Xtrain_pca, Xtest_pca, ytrain, ytest)
}

#######################################################
# Neuralnet + ICA, loop through number of ICA components    
#######################################################

for (i in seq(10, 100, by = 10)) {
    icamod <- fastICA(nxtrain, n.comp = i)
    Strain_ica <- icamod$S
    Stest_ica <- (nxtest %*% icamod$K %*% icamod$W)
    run_nnet(Strain_ica, Stest_ica, ytrain, ytest)
}

#######################################################
# Neuralnet + KMeans + Distance to centers
#######################################################
# Calculate distance to centers for all 500 rows in training vector
# Initialize training data matrix as all zeros.

for (nc in seq(10, 100, by = 20)) {
    kmeans.clusters <- kmeans(nxtrain, nc)
    nr_train <- nrow(nxtrain)
    nr_test <- nrow(nxtest)
    # nc <- nrow(kmeans.clusters$centers)
    Xtrain_km <- matrix(numeric(nr_train * nc), nrow = nr_train, ncol = nc)
    Xtest_km <- matrix(numeric(nr_test * nc), nrow = nr_test, ncol = nc)
    
    for (i in seq(1, nc)) {
        Cx <- kmeans.clusters$centers[i, ]
        Sx_train <- sweep(nxtrain, 2, Cx, "-")
        Sx_test <- sweep(nxtest, 2, Cx, "-")
        Dx_train <- apply(Sx_train, 1, function(r){ sqrt(sum(r^2))})
        Dx_test <- apply(Sx_test, 1, function(r){ sqrt(sum(r^2))})
        Xtrain_km[, i] <- Dx_train
        Xtest_km[, i] <- Dx_test
    }
    run_nnet(Xtrain_km, Xtest_km, ytrain, ytest)
}
