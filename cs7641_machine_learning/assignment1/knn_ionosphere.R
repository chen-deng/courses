## Clear workspace
rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

## Get a sample of training data
source("utils.R")
load_ionosphere()

## Function to run model 
run_model <- function(xtrain, xtest, ytrain, ytest, klist){
    # initialize percentage error
    perr = numeric(length=length(klist))
    
    N = length(klist)    
    for (i in 1:N) {
        
        # Perform the classification
        knn_model <- knn(xtrain, xtest, ytrain, k=klist[i])
        
        # Get error rate
        perr[i] <- 100 * sum(knn_model != ytest)/length(ytest)
        cat(sprintf("k = %d, error = %0.2f\n", klist[i], perr[i]))
    }
    perr
}

# KNN is contained in the 'class' library
library(class)

# Create list of neighbors
maxk <- 25
klist <- c(1 : maxk)

## Train the model
training_error <- run_model(rtrain$x, rtrain$x, 
                            rtrain$y, rtrain$y, klist)

test_error <- run_model(rtrain$x, rtest$x, 
                        rtrain$y, rtest$y, klist)

write.csv(data.frame(neighbors=klist, training_error=training_error, test_error=test_error),
          file="results/ionosphere/knn/error_vs_neighbors.csv",
          row.names=F)

