#include <sys/shm.h>
#include <unistd.h>
#include "../../common/constantes.h"
#include "../../common/colors.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("%sINICIO DE CINE TIMEOUT\n", KRED);
    sleep(60);
}