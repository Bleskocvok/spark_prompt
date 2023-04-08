
# spark-prompt

syntax example
```c
"[ ' ' \\exit(✓, ×) ] >> [ {white;5,82,158} \\username() ] :> [ {white;4, 56, 107} '@' \\hostname ] >> [ {255,255,255;5,82,158} \\pwd ] :> "

"[ ' ' \\exit(✓, ×) ] >> [ {white;0,91,187} \\username()] :> [ {0,0,0;255,213,0} \\hostname ] >> [ {255,255,255;5,82,158} \\pwd ] >>[ {white;0,91,187} ]:>[ {0,0,0;255,213,0} ]:> "
```



```
\a x b = a x b
[
    ~ { if exit then #FFfFF;#000000 else #000000;#FFfFF ; bold } ~
    (if exit then 'x' else 'v')
] >>
[ ~ { white   ; 5,82,158   }     username ~ ] :>
[ ~ { white   ; 4, 56, 107 } '@' hostname ~ ] >>
[ ~ { #ffffff ; 5,82,158   }     pwd      ~ ] :> 
```



```
[ { #0011FF (rgb 255 0 123) bold } "" ] :>
(segment - "" >>)
(hostname) (pwd) (if (exit) #ffeedd )
```
