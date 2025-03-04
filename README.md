# spark-prompt

[![Build & Tests](https://github.com/Bleskocvok/spark_prompt/actions/workflows/build-test.yml/badge.svg)](https://github.com/Bleskocvok/spark_prompt/actions/workflows/build-test.yml)
[![Clang-Tidy](https://github.com/Bleskocvok/spark_prompt/actions/workflows/tidy.yml/badge.svg)](https://github.com/Bleskocvok/spark_prompt/actions/workflows/tidy.yml)
[![Sanitizers](https://github.com/Bleskocvok/spark_prompt/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/Bleskocvok/spark_prompt/actions/workflows/sanitizers.yml)
[![Valgrind](https://github.com/Bleskocvok/spark_prompt/actions/workflows/valgrind.yml/badge.svg)](https://github.com/Bleskocvok/spark_prompt/actions/workflows/valgrind.yml)

This is very much a case of **WORK IN PROGRESS**.

Parser and “interpreter” of a lisp-inspired DSL (domain-specific language) intended for making the process of writing colorful `bash` prompts (or shell prompts in general) more sane. It aims to automate and abstract away the need to write color escape codes and escape sequences escaping those (yes, there are **two** levels of escaping to this).

The goal is to provide a long list of of useful functions to make prompt customizability easy.

Syntax example:
```
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') >> ]
[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') > ]
[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]
[ { #ffffff #05529E '' } (fmt ' ' (pwd_limited 35) ' ') >> ]
[ { #ffffff #005BBB '' } '' > ]
[ { #000000 #FFD500 '' } '' > ]
```

## Installation

For current user:

```sh
git clone https://github.com/Bleskocvok/spark_prompt
cd spark_prompt
make all
bash ./install
```

Or alternatively system-wide:

```sh
git clone https://github.com/Bleskocvok/spark_prompt
cd spark_prompt
make all
sudo bash ./system_install
```

## TODO

- Print grammar with -h
- Basic builtin functions
- Advanced builting functiona (git branch)
- Custom finctiondefinition
- Bake option
- More output types (powershell, zsh, ghci, …)
