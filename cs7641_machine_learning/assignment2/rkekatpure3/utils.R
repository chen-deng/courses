load_ionosphere <- function() {
    rtrain <<- list()
    rtest <<- list()
    df <- read.csv("ionosphere/ionosphere_data.csv", header=F, sep=",")
    rtrain$x <<- as.matrix(df[1:151, -35])
    rtrain$y <<- as.factor(df[1:151, 35])
    
    rtest$x <<- as.matrix(df[151:351, -35])
    rtest$y <<- as.factor(df[151:351, 35])
}





