The folder contains 1st homework in Physical Design for Nanometer Ics (NTU)
at 2021-2 semeter, with the following files:  
-	Course Problem File: prog1_partitioning.pdf
-	Source Code    Folder: src/
-       Report         File: report.pdf
-       Executive      File: bin/fm
-	InputData      Folder: input_pa1/
-	OutputData     Folder: output/

This problem is to partition a group of cells with minimum cutting size.
We can use Makefile to generate .fm in /bin, and operate it.
For example:
	./bin/fm input_pa1/input_1.dat output/output_1.dat

-----------------------------------------------------------------------------------

這個資料夾包含了奈米積體電路實體設計(台大課程)在2021-2學期的第一次作業, 有下列的檔案:
-	授課題目檔: prog1_partitioning.pdf
-	程式作業檔: src/
-       作業報告檔: report.pdf
-       程式執行檔: bin/fm
-	輸入測資檔: input_pa1/

這個問題主要是將一群元件用演算法去做切割，以得到最低的Cutsize.
我們可以使用Makefile在/bin內生成.fm, 並且執行這個程式
舉例來說:
	./bin/fm input_pa1/input_1.dat output/output_1.dat
