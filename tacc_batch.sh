#!/bin/bash
#SBATCH -J concurrent_unbounded_queue # job name
#SBATCH -o result-tacc-%j.txt # output and error file name (%j=jobID)
#SBATCH -n 16 # total number of cpus requested
#SBATCH -p development # queue -- normal, development, etc.
#SBATCH -t 00:05:00 # run time (hh:mm:ss) - 5 minutes
#SBATCH --mail-user=siming.liu@utdallas.edu
#SBATCH --mail-type=begin # email me when the job starts
#SBATCH --mail-type=end # email me when the job finishes

${HOME}/multicore_project_3/src/concurrent_unbounded_queue 32 2000 16 9
