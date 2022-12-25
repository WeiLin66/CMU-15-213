# Shell Lab實驗紀錄

## 簡介

本實驗要求我們編些一個供使用者交互的終端介面，類似於Linux中的shell終端。實驗中首先我們需要先解析輸入指令(這些指令可能包含多個參數)，且指令支持進程背景執行。

另一個重點是該實驗模擬Unix系統中的作業控制(Job Control)，對創建的進程進行監控與管理(增、刪、改、查)。同時為了避免交互過程產生殭屍進程(Zombie)，我們需要編寫特殊的信號處理函式，並且該終端也要可以被Ctrl + C，Ctrl + Z退出程序和暫停程序。

細節部分可以參考資料夾中的`tshlab.pdf`中的`The tsh Specification`；環節程式編寫方面可以參考`shlab.pdf`

## 主結構

### 主程式架構

### Jobs Control

### 前臺與後臺進程

## 避免race condition

## 目標編寫函式

### 指令解析

### built-in command部分

### handlers部分

## csapp.c參考範例

## 如何進行驗證





