#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#include <stdio.h>
#include <memory.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;
#define BITS_PER_WORD 32
#define MASK 0x1f
#define SHIFT 5
//ÖÃ1
void set(int i,int a[]) {//ÖÃ1
	a[i >> SHIFT] |= 1 << (i & MASK);

}
//ÖÃ0
void clr(int i, int a[]) {
	a[i >> SHIFT] &= ~(1 << (i & MASK));
}
//²éÑ¯
int test(int i, int a[]) {
	return (a[i >> SHIFT] & (1 << (i & MASK)));
}
