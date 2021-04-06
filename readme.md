# Operating System (CE3002A) - Program 1
## fork() and shell
###### tags: `作業系統`, `109-2`

<!-- [TOC] -->
> 抱歉，我真的不會寫程式QQAQ

## 開發環境
- Ubuntu 20.04.2 LTS
- gcc/g++ (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0

## Q1
### 問題
1. 問題描述
請寫一隻程式驗證 fork() 的機制。驗證的內容包含 (1) 父子程序的 fork() 的返回值以及 (2) 如何控制父子程序的執行順序。
(P.S. 只有 Unix-based 的作業系統才有 fork() 等函式，所以請直接或虛擬機安裝相關的作業統，如 Ubuntu, Fedora 等等。請不要在 Windows 上做，會做不出來。)
2. 給分標準
(1) 10%，輸出父子程序各自的返回值。
(2) 20%，請找到一個方法去控制父子程序的執行順序，並且寫出一個會利用這個控制方法的功能。(P.S 只要有用到就會給分數，不需要寫的很複雜)

### 答案
#### 程式邏輯
由使用者先輸入想要讓父程序 or 子程序先執行
之後，在 `fork()` 後由父子自己去決定要等誰（使用`waitpid`）
父程式的功能是用瀏覽器開一個很ㄎ一ㄤ的網站（我在某次OS課寫的）
子程式的功能是等三秒

##### 有關父子 PID
在fork完之後
- `fork()`回傳值是：父程式即為自己pid / 子程式為 `0`
- `getpid()` 父程式的pid
- `getppid()` 子程式的pid

#### 執行結果
- Child First
![](https://i.imgur.com/p7wJ7Ga.png)

- Parent First
![](https://i.imgur.com/c2pMsoT.png)


---

## Q2
### 問題
1. 問題描述
請寫出一個 shell。
另外還有三個功能分別是 Pipe (|), Redirection to File (>), Redirection from File(<)。
2. 給分標準
(1) 30%，寫出基本 shell，能夠根據你的程式呼叫並執行各種 Linux 的指令，包含各項指令的參數。
(2) 10%，以你的 shell 為基礎寫出 pipe 的功能。如果你的程式只能跑一個pipe 只會得到 5%，必須要能夠 pipe 無數個指令才能得到完整的分數。
如以下範例：
    I. `ls | head -3`，5%
    II. `ls | head -3 | tail -1 | <other command> | …`，10%
(3) 10 %，以你的 shell 為基礎寫出 redirection to file 的功能。同(2)，必須要能夠連續重導入才有完整的分數。
(4) 10 %，以你的 shell 為基礎寫出 redirection from file 的功能。同(2)，必須要能夠連續重導出才有完整的分數。
(5) 10%，能夠組合 (2)、(3)、(4) 的功能。如範例：ls -al | head -3 > output

### 答案
#### 程式邏輯
<!-- 我們一步一步來 -->
##### Variables

##### Input and parsing
- 在程式的一開始我們先處理使用者輸入(`getline`)
    - 如果輸入直接為`exit`，則直接結束程式
- 接著我們把輸入的指令們做拆解
    - 利用`stringstream`從每個空白拆開，存到一個 vector (`cmd`)
    - 紀錄每個`|`的位置，存到另一個 vector (`vbar_pos`)

##### Run Pipes
- 把剛剛拆出來的 commands 逐個檢視
    - `this_vbar`為此 pipe「之後」的`|`
    - `last_vbar`為此 pipe「之前」的`|`
    - 如果是最後一個參數則不會再次更新
- 做兩個`pipe`：`p1`和`p2`
    - aaa
    - bbb
- 開個`fork`，因為一旦執行`exec()`系列函數，控制權就不在我們這了
    - 父程序準備I/O，並等待子程序完成
    - 子程序跑 command

##### Execute Command
- 這裡我們分成兩種情況，第一種是已經來到 pipe 末端
    - I/O
        - 把`p1`輸入接上子程序的標準輸入
        - `p2`就不要理他，直接關
        - 輸出就是我們要的結果，所以把子程序的標準輸出接上主程序的標準輸出
    - 處理參數
        - 利用前面的`this_vbar`跟`cmd`末端來拿到參數
        - 把參數加到`params`，以符合後面會用到的`exec`函數規則
        - 遇到特殊符號`>`與`<`，處理 Redirect to file
    - 執行命令
- 而另一種，也就是管線的頭或中間
    - I/O
        - 一樣把`p1`輸入接上子程序的標準輸入
        - 把`p2`輸出接上子程序的標準輸出
    - 處理參數
        - 利用前面的`last_vbar`跟`this_vbar`，從他們之間來拿到參數
        - 一樣，把參數加到`params`，以符合後面會用到的`exec`函數規則
    - 執行命令


#### 執行結果
- 基本的命令 `ls -al`
![](https://i.imgur.com/ibNoAID.png)

- 一個pipe
![](https://i.imgur.com/Fhfgh2J.png)

- 更多pipe
![](https://i.imgur.com/aj4iy5O.png)

- redirection to files
![](https://i.imgur.com/QIVXuIv.png)
<!-- ![](https://i.imgur.com/f0j0pZW.png) -->

- redirection from files
![](https://i.imgur.com/Ir29g3J.png)

- 大雜燴
![](https://i.imgur.com/eRqL0Gc.png)

- ERROR CODE REPORTED
![](https://i.imgur.com/cQmZ1Gl.png)



## References
- https://burweisnote.blogspot.com/2017/10/pipe.html