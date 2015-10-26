library(GA)

fitness <- function(x){
    # - abs(x) - cos(x)
    A = 10
    N = length(x)
    - (A * N + x * x - A * cos(2 * pi * x))
}

curve(fitness, -5, 5, 1000)
dimn <- 100
GA <- ga(type = "real-valued", fitness = fitness, 
         min = rep(-5, dimn), max = rep(5, dimn),
         pmutation=0.99,
         pcrossover=0.99)

summary(GA)
plot(GA)
# abline(v = GA@solution, lty = 3)