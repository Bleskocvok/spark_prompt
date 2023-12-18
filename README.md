
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

## Installation

```sh
git clone https://github.com/Bleskocvok/spark_prompt
cd spark_prompt
make all
bash ./nosudo_install
```

Or alternatively:

```sh
git clone https://github.com/Bleskocvok/spark_prompt
cd spark_prompt
make all
sudo bash ./install
```
