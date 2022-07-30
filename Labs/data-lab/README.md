## 實驗環境搭建

| *建議使用Ubuntu作業系統或其他Linux distros*

1. 首先從Github拉取該repo
2. 複製Lab_Backup中的Lab檔
3. 軟體配置
   1. 安裝gcc: `sudo apt-get install gcc`(已安裝請跳過)
   2. 安裝make: `sudo apt -y install make`(已安裝請跳過)
4. 編寫腳本(可選)

⚠️使用腳本的目的是為了省略`make`, `make` `clean`, `./btest`

開啟lab並在資料夾中建立`run.sh，`，並將下列程式碼複製到文件中

```bash
#/bin/bash
make clean
make
./btest
```

5. 完成後每次編譯只需要在終端中輸入`./run.sh即可`
6. `./dlc -e bits.c`查看執行operator的次數