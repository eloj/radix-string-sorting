OPT=-O3 -fomit-frame-pointer -funroll-loops -fstrict-aliasing -march=native -mtune=native -msse4.2 -mavx
LTOFLAGS=-flto -fno-fat-lto-objects -fuse-linker-plugin
WARNFLAGS=-Wall -Wextra -Wshadow -Wstrict-aliasing -Wcast-qual -Wcast-align -Wpointer-arith -Wredundant-decls -Wfloat-equal -Wswitch-enum
CWARNFLAGS=-Wstrict-prototypes -Wmissing-prototypes
MISCFLAGS=-fstack-protector -fvisibility=hidden
DEVFLAGS=-Wno-unused-parameter -Wno-unused-variable -DVERIFY -DBUCKETUSE

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

ifdef MEMCHECK
	TEST_PREFIX:=valgrind --tool=memcheck --leak-check=full --track-origins=yes
endif

ifdef PERF
	TEST_PREFIX:=perf stat
endif

ifdef PROFILEGEN
	MISCFLAGS+=-fprofile-generate
	OPTIMIZED=y
endif

ifdef PROFILEUSE
	MISCFLAGS+=-fprofile-use
	OPTIMIZED=y
endif

ifdef STACKCHECK
	MISCFLAGS+=-fcallgraph-info=su
endif

ifdef LTO
	MISCFLAGS+=${LTOFLAGS}
endif

ifndef OPTIMIZED
	MISCFLAGS+=-g -DDEBUG $(DEVFLAGS)
else
	MISCFLAGS+=-DNDEBUG
endif

CFLAGS=-std=c11 $(OPT) $(CWARNFLAGS) $(WARNFLAGS) $(MISCFLAGS)
CXXFLAGS=-std=gnu++17 -fno-rtti $(OPT) $(WARNFLAGS) $(MISCFLAGS)

.PHONY: clean backup

all: sort_ce0

test: all
	$(TEST_PREFIX) ./sort_ce0

sort_ce0: sort_ce0.c

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

backup:
	tar -cJf ../$(notdir $(CURDIR))-`date +"%Y-%m"`.tar.xz ../$(notdir $(CURDIR))

clean:
	rm -f sort_ce0 vgcore.* core.* *.gcda
