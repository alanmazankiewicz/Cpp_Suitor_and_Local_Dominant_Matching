## 3.4 ## Quality

library(dplyr)
library(reshape2)
library(ggplot2)


std_dyn_1 <- read.table("3.4_3.5/SeqDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_1$Runtime <- NULL
std_dyn_2 <- read.table("3.4_3.5/SeqSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_2$Runtime <- NULL
std_dyn_3 <- read.table("3.4_3.5/ParDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_3$Runtime <- NULL
std_dyn_3$Threads <- NULL
std_dyn_4 <- read.table("3.4_3.5/ParSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_4$Runtime <- NULL
std_dyn_4$Threads <- NULL
std_dyn_5 <- read.table("3.4_3.5/ParFreeSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_5$Runtime <- NULL
std_dyn_5$Threads <- NULL

std_dyn <- rbind(std_dyn_1, std_dyn_2, std_dyn_3, std_dyn_4, std_dyn_5)
std_dyn_sumar <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(mean))
std_dyn_sumar <-  melt(std_dyn_sumar, id.vars='Algorithm')

(ggplot(std_dyn_sumar, aes(variable, value, fill = Algorithm)) 
  + geom_bar(position = position_dodge(), stat = "identity")
  + ggtitle("Quality Comparison") + ylab("Quality") 
  + theme(plot.title = element_text(hjust = 0.5))
  + scale_fill_brewer(palette="YlOrBr"))
ggsave("3.4_3.5/Quality_All.pdf")

std_dyn_sumar_heavy <- filter(std_dyn_sumar, Algorithm == "SeqDomiImp" | Algorithm == "SeqSuitorImp" | Algorithm == "ParSuitorImp")
(ggplot(std_dyn_sumar_heavy, aes(variable, value, fill = Algorithm)) 
  + geom_bar(position = position_dodge(), stat = "identity")
  + coord_cartesian(ylim=c(std_dyn_sumar_heavy$value[1] * 0.999, std_dyn_sumar_heavy$value[1] * 1.0001))
  + ggtitle("Quality Comparison Heavy Matchings scaled") + ylab("Quality") 
  + theme(plot.title = element_text(hjust = 0.5))
  + scale_fill_brewer(palette="YlOrBr"))
ggsave("3.4_3.5/Quality_HeavyMatchings_scaled.pdf")

std_dyn_sumar_simp <- filter(std_dyn_sumar, Algorithm != "SeqDomiImp" & Algorithm != "SeqSuitorImp" & Algorithm != "ParSuitorImp")
(ggplot(std_dyn_sumar_simp, aes(variable, value, fill = Algorithm)) 
  + geom_bar(position = position_dodge(), stat = "identity")
  + coord_cartesian(ylim=c(std_dyn_sumar_simp$value[1] * 0.9999, std_dyn_sumar_simp$value[1] * 1.0001))
  + ggtitle("Quality Comparison Simple scaled") + ylab("Quality") 
  + theme(plot.title = element_text(hjust = 0.5))
  + scale_fill_brewer(palette="YlOrBr"))
ggsave("3.4_3.5/Quality_Simple_scaled.pdf")

# 3.5 

std_dyn_1 <- read.table("3.4_3.5/SeqDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_2 <- read.table("3.4_3.5/SeqSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_3 <- read.table("3.4_3.5/ParDomi.txt", header = T, sep = " ", comment.char = "#")
std_dyn_3 <- filter(std_dyn_3, Threads == 32)
std_dyn_3$Threads <- NULL
std_dyn_4 <- read.table("3.4_3.5/ParSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_4 <- filter(std_dyn_4, Threads == 32)
std_dyn_4$Threads <- NULL
std_dyn_5 <- read.table("3.4_3.5/ParFreeSuitor.txt", header = T, sep = " ", comment.char = "#")
std_dyn_5 <- filter(std_dyn_5, Threads == 32)
std_dyn_5$Threads <- NULL

std_dyn <- rbind(std_dyn_1, std_dyn_2, std_dyn_3, std_dyn_4, std_dyn_5)
std_dyn_sumar <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(mean))
(ggplot(std_dyn_sumar, aes(x = Runtime, y = Quality, color = Algorithm)) 
  + geom_point() 
  + ggtitle("Runtime Vs Quality") + xlab("Runtime in s") 
  + theme(plot.title = element_text(hjust = 0.5)))
ggsave("3.4_3.5/3.5_all.pdf")

std_dyn_sumar_heavy <- filter(std_dyn_sumar, Algorithm == "SeqDomiImp" | Algorithm == "SeqSuitorImp" | Algorithm == "ParSuitorImp")
(ggplot(std_dyn_sumar_heavy, aes(x = Runtime, y = Quality, color = Algorithm)) 
  + geom_point()
  + ggtitle("Runtime Vs Quality (Heavy only)") + xlab("Runtime in s") 
  + theme(plot.title = element_text(hjust = 0.5)))
ggsave("3.4_3.5/3.5_heavyOnly.pdf")

std_dyn_sumar_no_heavy <- filter(std_dyn_sumar, Algorithm != "SeqDomiImp" & Algorithm != "SeqSuitorImp" & Algorithm != "ParSuitorImp")
(ggplot(std_dyn_sumar_no_heavy, aes(x = Runtime, y = Quality, color = Algorithm)) 
  + geom_point()
  + ggtitle("Runtime Vs Quality (Simple only)") + xlab("Runtime in s") 
  + theme(plot.title = element_text(hjust = 0.5)))
ggsave("3.4_3.5/3.5_simpleOnly.pdf")



