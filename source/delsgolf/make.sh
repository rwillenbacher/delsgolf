#/bin/bash

TMP="./tmp"
#-fomit-frame-pointer 
ADDITIONALCFLAGS="-Wall -W -mregparm=5 -Wwrite-strings -ffunction-sections -fdata-sections -DUSE_TI89 -DOPTIMIZE_CALC_CONSTS -DMIN_AMS=205 -mno-bss -DSAVE_SCREEN"
ADDITIONALASFLAGS="-WA,-t -Wa,-l"
ADDITIONALLDFLAGS="--optimize-relocs --optimize-nops --optimize-returns --optimize-branches --optimize-moves --optimize-tests --optimize-calcs --remove-unused --cut-ranges --reorder-sections --merge-constants"

ADDITIONALFLAGS="${ADDITIONALCFLAGS} ${ADDITIONALASFLAGS} ${ADDITIONALLDFLAGS}"


tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/draw.o draw.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/draw.S draw.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/entities.o entities.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/entities.S entities.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/map.o map.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/map.S map.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/mathlib.o mathlib.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/mathlib.S mathlib.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/render.o render.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/render.S render.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/repository.o repository.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/repository.S repository.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/soundfx.o soundfx.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/soundfx.S soundfx.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/sprites.o sprites.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/sprites.S sprites.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/textures.o textures.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/textures.S textures.c
tigcc.exe -pipe -c -Os ${ADDITIONALFLAGS} -o ${TMP}/main.o main.c
tigcc.exe -pipe -S -Os ${ADDITIONALFLAGS} -o ${TMP}/main.S main.c

tigcc.exe -pipe -c ${ADDITIONALFLAGS} -o ${TMP}/vspan.o vspan.s
tigcc.exe -pipe -c ${ADDITIONALFLAGS} -o ${TMP}/hspan.o hspan.s
tigcc.exe -pipe -c ${ADDITIONALFLAGS} -o ${TMP}/sspan.o sspan.s
tigcc.exe -pipe -c ${ADDITIONALFLAGS} -o ${TMP}/fpmath.o fpmath.s

tigcc.exe ${ADDITIONALFLAGS} -v -v0 -o delsgolf -n delsgolf ${TMP}/draw.o ${TMP}/entities.o ${TMP}/map.o ${TMP}/mathlib.o ${TMP}/render.o ${TMP}/repository.o ${TMP}/soundfx.o ${TMP}/sprites.o ${TMP}/textures.o ${TMP}/main.o ${TMP}/vspan.o ${TMP}/hspan.o ${TMP}/sspan.o ${TMP}/fpmath.o
