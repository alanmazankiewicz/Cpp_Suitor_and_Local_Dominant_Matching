## Plots for 3.2 ##

library(dplyr)
library(reshape2)
library(ggplot2)

## Runtime of SeqDomi vs SeqSuitor on larger Graph (-nVertices 1000000 -nEdges 100000000)


  std_dyn_1 <- read.table("3.2/SeqDomi.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_1$Quality <- NULL
  std_dyn_2 <- read.table("3.2/SeqSuitor.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_2$Quality <- NULL
  std_dyn <- rbind(std_dyn_1, std_dyn_2)
  std_dyn_sumar <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(mean))
  std_dyn_sumar <-  melt(std_dyn_sumar, id.vars='Algorithm')
  colnames(std_dyn_sumar)[3] <- "avg"
  
  std_com_const <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(sd))
  std_com_const <- melt(std_com_const, id.vars='Algorithm')
  colnames(std_com_const)[3] <- "std"
  std_dyn_sumar <- full_join(std_dyn_sumar, std_com_const, by = c("Algorithm", "variable"))
  
  (ggplot(std_dyn_sumar, aes(variable, avg, fill = Algorithm)) 
    + geom_bar(position = position_dodge(), stat = "identity")  
    + geom_errorbar(aes(ymin = avg-std, ymax = avg+std), position = position_dodge(0.9), width = 0.5)
    + ggtitle("SeqDomi vs SeqSuitor on -nVertices 1000000 -nEdges 100000000") + ylab("mean [s]") 
    + theme(plot.title = element_text(hjust = 0.5))
    + scale_fill_brewer(palette="YlOrBr"))
  ggsave("3.2/SeqDomi_vs_SeqSuitor.pdf")



## Runtime of SeqDomi vs SeqSuitor vs Heavy Matchign 

  std_dyn_1 <- read.table("3.2/SeqDomi_2.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_1$Quality <- NULL
  std_dyn_2 <- read.table("3.2/SeqSuitor_2.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_2$Quality <- NULL
  
  std_dyn <- rbind(std_dyn_1, std_dyn_2)
  std_dyn_sumar <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(mean))
  std_dyn_sumar <-  melt(std_dyn_sumar, id.vars='Algorithm')
  colnames(std_dyn_sumar)[3] <- "avg"
  
  std_com_const <- std_dyn %>% group_by(Algorithm) %>% summarise_each(funs(sd))
  std_com_const <- melt(std_com_const, id.vars='Algorithm')
  colnames(std_com_const)[3] <- "std"
  std_dyn_sumar <- full_join(std_dyn_sumar, std_com_const, by = c("Algorithm", "variable"))
  
  (ggplot(std_dyn_sumar, aes(variable, avg, fill = Algorithm)) 
    + geom_bar(position = position_dodge(), stat = "identity")  
    + geom_errorbar(aes(ymin = avg-std, ymax = avg+std), position = position_dodge(0.9), width = 0.5)
    + ggtitle("Runtime SeqDomi vs SeqSuitor (Simp) vs Heavy Matchings (Imp)") + ylab("mean [s]") 
    + theme(plot.title = element_text(hjust = 0.5))
    + scale_fill_brewer(palette="YlOrBr"))
  ggsave("3.2/SeqDomi_vs_SeqSuitor_vs_Heavy.pdf")

## Scaling
  std_dyn_1 <- read.table("3.2/SeqDomi_3.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_1$Quality <- NULL
  std_dyn_2 <- read.table("3.2/SeqSuitor_3.txt", header = T, sep = " ", comment.char = "#")
  std_dyn_2$Quality <- NULL
  std_dyn <- rbind(std_dyn_1, std_dyn_2)
  std_dyn_sumar <- std_dyn %>% group_by(Algorithm, Section) %>% summarise_each(funs(mean))
  
  (ggplot(std_dyn_sumar, aes(x = Section, y = Runtime, color = Algorithm)) 
    + geom_line() 
    + geom_point()
    + ggtitle("Suitor vs Dominant Scaling Graphs") + ylab("Mean Runtime [s]") 
    + theme(plot.title = element_text(hjust = 0.5)))
ggsave("3.2/Scaling_Graphs.pdf")

### BONUS

## Runtime of Heavy Matchings (Bonus)
sty_dyn_sumar_heavy <- filter(std_dyn_sumar, Algorithm == "SeqDomiImp" | Algorithm == "SeqSuitorImp")
(ggplot(sty_dyn_sumar_heavy, aes(variable, avg, fill = Algorithm)) 
  + geom_bar(position = position_dodge(), stat = "identity")
  + coord_cartesian(ylim=c(sty_dyn_sumar_heavy$avg[1] * 0.9, sty_dyn_sumar_heavy$avg[1] * 1.1))
  + ggtitle("Bonus: Runtime Sequntiell Heavy Matchings Dominant vs Suitor") + ylab("mean [s]") 
  + theme(plot.title = element_text(hjust = 0.5))
  + scale_fill_brewer(palette="YlOrBr"))
ggsave("3.2/HeavyMatchings.pdf")

## Bonus
# std_dyn_3 <- read.table("3.2/ParSuitor_2.txt", header = T, sep = " ", comment.char = "#")
# std_dyn_3$Quality <- NULL
# std_dyn_3 <- filter(std_dyn_3, Threads == 32)
# std_dyn_3$Algorithm <- "ParSuitorImp32"
# std_dyn_3$Threads <- NULL
# 
# std_dyn_sumar3 <- std_dyn_3 %>% group_by(Algorithm) %>% summarise_each(funs(mean))
# std_dyn_sumar3 <-  melt(std_dyn_sumar3, id.vars='Algorithm')
# colnames(std_dyn_sumar3)[3] <- "avg"
# 
# std_com_const3 <- std_dyn_3 %>% group_by(Algorithm) %>% summarise_each(funs(sd))
# std_com_const3 <- melt(std_com_const3, id.vars='Algorithm')
# colnames(std_com_const3)[3] <- "std"
# std_dyn_sumar3 <- full_join(std_dyn_sumar3, std_com_const3, by = c("Algorithm", "variable"))