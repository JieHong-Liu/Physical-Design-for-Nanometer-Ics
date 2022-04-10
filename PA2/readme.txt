The folder contains 1st homework in Physical Design for Nanometer Ics (NTU)
at 2021-2 semeter, with the following files:  
-	Course Problem File: prog2_floorplanning.pdf
-	Source Code    Folder: src/
- Report         File: report.pdf
- Executive      File: bin/fp
-	InputData      Folder: input_pa2/
-	OutputData     Folder: output_pa2/

Coding problem is to floorplan a group of macros with minimum cost(wire-length+frame-size).
We can use Makefile to generate .fp in /bin, and operate it.
For example:
	./bin/fp 0.5 input_pa2/test.block input_pa2/test.nets output_pa2/test.rpt

-----------------------------------------------------------------------------------


這個資料夾包含了奈米積體電路實體設計(台大課程)在2021-2學期的第二次作業, 有下列的檔案:
-	授課題目檔: prog2_floorplanning.pdf
-	程式作業檔: src/
- 作業報告檔: report.pdf
- 程式執行檔: bin/fp
-	輸入測資檔: input_pa2/
- 輸出測資檔: output_pa2/

程式問題是要排列一群巨集元件用最小的成本(繞線長度+框架尺寸).
我們可以使用Makefile在/bin內生成.fp, 並且執行這個程式.
舉例來說:
	./bin/fp 0.5 input_pa2/test.block input_pa2/test.nets output_pa2/test.rpt