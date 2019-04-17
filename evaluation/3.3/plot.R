## Plots for 3.3 and 3.4 ##

library(dplyr)
library(reshape2)
library(ggplot2)

## Runtime 
std_dyn_1 <- read.table("3.3/SeqDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_1$Quality <- NULL
std_dyn_2 <- read.table("3.3/SeqSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_2$Quality <- NULL
std_dyn_3 <- read.table("3.3/ParDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_3$Quality <- NULL
std_dyn_4 <- read.table("3.3/ParSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_4$Quality <- NULL
std_dyn_5 <- read.table("3.3/ParFreeSuitorGood.txt", header = T, sep = " ", comment.char = "#")
std_dyn_5$Quality <- NULL

std_dyn_base <- rbind(std_dyn_1, std_dyn_2)
std_dyn_base_sumar <- std_dyn_base %>% group_by(Algorithm) %>% summarise_each(funs(mean))

std_dyn <- rbind(std_dyn_3, std_dyn_4, std_dyn_5)
std_dyn_sumar <- std_dyn %>% group_by(Algorithm, Threads) %>% summarise_each(funs(mean))
std_dyn_sumar$Sequential <- 0

for(i in 1:nrow(std_dyn_sumar)) {
  if(std_dyn_sumar[i, "Algorithm"] == "ParDomi"){
    std_dyn_sumar[i, "Sequential"] = filter(std_dyn_base_sumar, Algorithm == "SeqDomiSimp")$Runtime
  } else {
    std_dyn_sumar[i, "Sequential"] = filter(std_dyn_base_sumar, Algorithm == "SeqSuitorSimp")$Runtime
  }
}

std_dyn_sumar$Speedup = std_dyn_sumar$Sequential / std_dyn_sumar$Runtime
std_dyn_sumar$Efficiency = std_dyn_sumar$Speedup / std_dyn_sumar$Threads
std_dym_sumar_noFree <- filter(std_dyn_sumar, Algorithm != "ParFreeSuitorSimp")

#Time
(ggplot(data = std_dyn_sumar, aes(x = Threads, y = Runtime, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Runtime vs nThreads in s") + expand_limits(y=0))
ggsave("3.3/Runtime.pdf")

#Time without Free
(ggplot(data = std_dym_sumar_noFree, aes(x = Threads, y = Runtime, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Runtime vs nThreads (no Free) in s") + expand_limits(y=0))
ggsave("3.3/Runtime_noFree.pdf")

# Speedup
(ggplot(data = std_dyn_sumar, aes(x = Threads, y = Speedup, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Speedup") + geom_abline() + expand_limits(y=0))
ggsave("3.3/Speedup.pdf")

# Sppedup NoFree
(ggplot(data = std_dym_sumar_noFree, aes(x = Threads, y = Speedup, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Speedup(no Free)") + expand_limits(y=0) + geom_abline())
ggsave("3.3/Speedup_noFree.pdf")

# Efficiency
(ggplot(data = std_dyn_sumar, aes(x = Threads, y = Efficiency, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Efficiency") + expand_limits(y=0))
ggsave("3.3/Efficiency.pdf")



### Bonus Runtime comparison of optimization on ParFreeSuitor
std_dyn_1 <- read.table("3.3/ParFreeSuitorGood.txt", header = T, sep = " ", comment.char = "#")
std_dyn_1$Quality <- NULL
std_dyn_1$Algorithm <-"ParFreeSuitorSimpNew"

std_dyn_2 <- read.table("3.3/ParFreeSuitorBad.txt", header = T, sep = " ", comment.char = "#")
std_dyn_2$Quality <- NULL
std_dyn_2$Algorithm <- "ParFreeSuitorSimpOld"

std_dyn <- rbind(std_dyn_1, std_dyn_2)
std_dyn_sumar <- std_dyn %>% group_by(Algorithm, Threads) %>% summarise_each(funs(mean))

#Time
(ggplot(data = std_dyn_sumar, aes(x = Threads, y = Runtime, group = Algorithm, colour = Algorithm)) + geom_line() 
  + geom_point()
  + ggtitle("Optimization of Lock Free Suitor") + expand_limits(y=0))
ggsave("3.3/Optimization_ParFree.pdf")

