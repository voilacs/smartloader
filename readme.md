README

•	This is an upgraded Simple Loader made in C which does not load any ELF executables upfront, instead loads them only when needed while the programme is under execution.
•	No segment is allocated upfront initially. The entry point address is typecasted and a direct attempt _start method is being made, which would obviously lead to Segmentation Fault.
•	The segmentation fault is handled by using SIGSEV(signal handler).SIGINFO is used to find the address of the element causing this fault.
•	A loop used to iterate through all the addresses of the segments. Using this loop a range is found in which the above mentioned address lies.
•	The given range belongs to a segment which is then loaded and allocated a space in the memory using mmap (Lazy process) and copy the content of the segment.
•	The programme simply resumes after the handling of this segmentation fault.
•	After the execution is complete the Simple Smart Loader prints the following:
(i)	the total number of page faults
(ii)	total number of page allocations carried out for that executable’s execution
(iii)	total amount of internal fragmentation in KB.
•	The programme performs error handling, including checking for file opening errors, memory allocation errors, and errors during reading and mapping the ELF file and segments.


Designs peculiar to our loader:
    -The main function calls the function that is responsible for calling the _start method of the given elf.
    -The function then loops through all phnum segments and allocates memorry to each one of segment after each iteration it then tries to call the _start method again resulting in seg fault or successful operation thereafter.
    -We then try to keep track of all the statistics to print them in the end


    Github Repo: https://github.com/voilacs/smartloader


Contributions:
Anmol(22083): The previous loader was allocating memory upfront. Seg fault handler was made to handle any seg faults that might come and jmp was incorporated to return control back to appropriate processes.

Dhawal(22160): Page faults were found and page allocations were counted from all the places as well as internal fragmentation was taken care of to ensure fair statistics.

