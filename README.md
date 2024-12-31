# spark-prompt

This is very much a case of **WORK IN PROGRESS**.

Parser and “interpreter” of a lisp-inspired DSL (domain-specific language) intended for making the process of writing colorful `bash` prompts (or shell prompts in general) more sane. It aims to automate and abstract away the need to write color escape codes and escape sequences escaping those (yes, there are **two** levels of escaping to this).

The goal is to provide a long list of of useful functions to make prompt customizability easy.

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
