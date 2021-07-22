# csvreader
Program to read and calculate CSV table

For example table 
![image](https://user-images.githubusercontent.com/38140268/126676712-3aca32a7-11fe-4912-95b3-e9b956d2bbf6.png)

that presented as

<pre>,A,B,Cell,&nbsp;
1,1,0,-1&nbsp;
2,2,=A1+Cell30,0&nbsp;
30,0,=B1+A1,5</pre>


will be transformed into 

<pre>,A,B,Cell&nbsp;
1,1,0,-1&nbsp;
2,2,6,0&nbsp;
30,0,1,5</pre>
