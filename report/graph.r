### load package
library(ggplot2)

### get parameter
options <- commandArgs(trailingOnly = TRUE)

name <- sub("*.txt", "", options)

### load data
time_data = read.table(options, header = TRUE, sep = ",", skip = 1)

### plot
attach(time_data)
setEPS()
postscript(paste(name, ".eps", sep = ""))
ggplot() +
  geom_line(data = time_data, aes(x = time_data$ThreadNumber, y = time_data$LockedQueue, colour = "LockedQueue")) +
  geom_line(data = time_data, aes(x = time_data$ThreadNumber, y = time_data$LockFreeQueue, colour = "LockFreeQueue")) +
  xlab('Thread Number') + ylab('Time (Nanosecond)')
dev.off()
