1. 學號：M11007444
2. 姓名：劉杰閎
3. 使用之程式語言：< C++ >
4. 使用之編譯平台：< Linux GNU g++ >
5. 檔案壓縮方式: <m11007444.tgz>
6. 各檔案說明：
	bin/fm				                : LINUX g++編譯之主程式執行檔
	src/        				          : source code
	prog1_partitioning.pdf 		    : 題目
	readme.txt	    	            : 本檔案
	input_pa1/				            : input directory.
	Makefile				              : Makefile
7.  編譯方式說明： 
    主程式：
	    在 m11007444/ 這個資料夾下指令 : make
	    即可產生 fm 執行檔
	
8. 執行、使用方式說明：
    主程式：
        compile 完成後，在 bin/ 目錄下會產生一個 fm 的執行檔
   	    執行檔的命令格式為 :
   	    ./fm [input file name] [output file name]
           
	    ex: ./fm input_pa1/input_0.dat output_0.dat
		產生出來的output會在目錄下
