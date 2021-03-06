# Load the MNIST digit recognition dataset into R
# http://yann.lecun.com/exdb/mnist/
# assume you have all 4 files and gunzip'd them
# creates train$n, train$x, train$y  and test$n, test$x, test$y
# e.g. train$x is a 60000 x 784 matrix, each row is one digit (28x28)
# call:  show_digit(train$x[5,])   to see a digit.
# brendan o'connor - gist.github.com/39760 - anyall.org

load_mnist <- function() {
    load_image_file <- function(filename) {
        ret = list()
        f = file(filename,'rb')
        readBin(f,'integer',n=1,size=4,endian='big')
        ret$n = readBin(f,'integer',n=1,size=4,endian='big')
        nrow = readBin(f,'integer',n=1,size=4,endian='big')
        ncol = readBin(f,'integer',n=1,size=4,endian='big')
        x = readBin(f,'integer',n=ret$n*nrow*ncol,size=1,signed=F)
        ret$x = matrix(x, ncol=nrow*ncol, byrow=T)
        close(f)
        ret
    }
    load_label_file <- function(filename) {
        f = file(filename,'rb')
        readBin(f,'integer',n=1,size=4,endian='big')
        n = readBin(f,'integer',n=1,size=4,endian='big')
        y = readBin(f,'integer',n=n,size=1,signed=F)
        close(f)
        y
    }
    rtrain <<- load_image_file('mnist/train-images-idx3-ubyte')
    rtest <<- load_image_file('mnist/t10k-images-idx3-ubyte')
    
    rtrain$y <<- load_label_file('mnist/train-labels-idx1-ubyte')
    rtest$y <<- load_label_file('mnist/t10k-labels-idx1-ubyte')  
}


show_digit <- function(arr784, col=gray(12:1/12), ...) {
    image(matrix(arr784, nrow=28)[,28:1], col=col, ...)
}

# Get a sample of training data
get_sample <- function(Nsamples, set) {
    ret <- list()
    max <- nrow(set$x)    
    indices <- sample(1 : max, Nsamples, replace=F)
    ret$x <- set$x[indices,]
    ret$y <- set$y[indices]
    ret 
}

load_ionosphere <- function() {
    rtrain <<- list()
    rtest <<- list()
    df <- read.csv("ionosphere/ionosphere_data.csv", header=F, sep=",")
    rtrain$x <<- as.matrix(df[1:151, -35])
    rtrain$y <<- as.factor(df[1:151, 35])
    
    rtest$x <<- as.matrix(df[151:351, -35])
    rtest$y <<- as.factor(df[151:351, 35])
}





