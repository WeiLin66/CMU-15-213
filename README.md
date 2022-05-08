## CSAPP

## 關於

這個repo是我自學CSAPP的一個紀錄，內容包括個章節的homeworks解答, Lab解答, 上課講義, 內容來自官方:

- [CMU student site](http://csapp.cs.cmu.edu/3e/home.htmlhttp://csapp.cs.cmu.edu/3e/labs.htmlhttps:)
- [CMU官方網站](http://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/index.htmlhttps:/)

## 如何使用

1. Labs資料夾: 存放個章節末的實驗解答
2. Labs_Backup: 初始的Lab
3. homeworks資料夾: 包含個章節的回家作業解答
4. resources: 上課講義與教科書
5. *essay: 規劃中，未來有文章講解會放在這裡，方便理解

## log

* [X] 講義整理
* [X] labs

  * [X] [*Data Lab*](http://csapp.cs.cmu.edu/im/labs/datalab.tar)
  * [ ] [*Bomb Lab*](http://csapp.cs.cmu.edu/im/labs/bomblab.tar)
  * [ ] [*Attack Lab*](http://csapp.cs.cmu.edu/im/labs/attacklab.tar)
  * [ ] [*Cache Lab*](http://csapp.cs.cmu.edu/im/labs/cachelab.tar)
  * [ ] [*Shell Lab*](http://csapp.cs.cmu.edu/im/labs/shlab.tar)
  * [ ] [*Malloc Lab*](http://csapp.cs.cmu.edu/im/labs/malloclab.tar)
  * [ ] [*Proxy Lab*](http://csapp.cs.cmu.edu/im/labs/proxylab.tar)
* [ ] homeworks

  * [ ] ch2(進行中...)
  * [ ] ch3
  * [ ] ch5
  * [ ] ch6
  * [ ] ch7
  * [ ] ch8
  * [ ] ch9
  * [ ] ch10
  * [ ] ch11
  * [ ] ch12

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

## 聯絡我

email: linhoway@gmail.com

[更多資源](https://equinox-mahogany-b74.notion.site/CMU-15-213-Intro-to-Computer-Systems-c0d76b0ec3f840ff8f5c46113a9b43c3)
