
# spark-prompt

Syntax example:
```
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') >> ]
[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') :> ]
[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]
[ { #ffffff #05529E '' } (fmt ' ' (pwd_limited 35) ' ') >> ]
[ { #ffffff #005BBB '' } '' :> ]
[ { #000000 #FFD500 '' } '' :> ]
```

