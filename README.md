# ProjectSO
Project Operating System 2018 - 2019

Step 1.
Develop a software that, given a path, scans all files in the file system in a recursive manner.
The proposed structure provides a pool of threads, one for each processor core.
The main thread starts by making an "ls" (list directory, but with C functions) to the current directory, then inserts the names of the found items into a list: In the list of strings will be stored files and directories with the absolute path. After the first entry in the list the other running threads access the list and each of these deletes a directory type element from the list, then scans (ls) the directory and inserts the new found items into the list.
Access to the list is protected by mutex.
The initial directory must be chosen dynamically by argument.
