#include <stdio.h>

/*
 * Maps the LED segment name onto the logical value in the
 * shift-register, for example on the LED with the pinout:
 *    E (1)
 *    D (2)
 *    . (3)
 *    C (4)
 *    G (5)
 *    B (7)
 *    F (10)
 *    A (11)
 * and given the logical pin names Q0 - Q7 on the shift-register
 * chip, this was the most convenient layout for connecting the SR
 * pins to the appropriate segment pins on the display.
 */
#define A 0
#define B 7
#define C 4
#define D 2
#define E 1
#define F 6
#define G 5
#define DP 3
#define NUM_SEGMENTS 8

/*
 * Digit masks. Bit-wise OR of the individual segment
 * masks that make up a digit. Covers up to hexadecimal
 * F.
 */
#define NUM_DIGIT_MASKS 0x0F+1

unsigned DIGIT_MASKS[NUM_DIGIT_MASKS];

#define m(seg) (1 << seg)

void init_digit_masks() {
  DIGIT_MASKS[0] = m(A)|m(B)|m(C)|m(D)|m(E)|m(F);
  DIGIT_MASKS[1] = m(B)|m(C);
  DIGIT_MASKS[2] = m(A)|m(B)|m(G)|m(E)|m(D);
  DIGIT_MASKS[3] = m(A)|m(B)|m(G)|m(C)|m(D);
  DIGIT_MASKS[4] = m(F)|m(G)|m(B)|m(C);
  DIGIT_MASKS[5] = m(A)|m(F)|m(G)|m(C)|m(D);
  DIGIT_MASKS[6] = m(A)|m(F)|m(G)|m(C)|m(D)|m(E);
  DIGIT_MASKS[7] = m(A)|m(B)|m(C);
  DIGIT_MASKS[8] = m(A)|m(B)|m(C)|m(D)|m(E)|m(F)|m(G);
  DIGIT_MASKS[9] = m(A)|m(F)|m(G)|m(B)|m(C)|m(D);
  DIGIT_MASKS[0xA] = m(A)|m(F)|m(B)|m(G)|m(E)|m(C);
  DIGIT_MASKS[0xb] = m(F)|m(G)|m(E)|m(C)|m(D);
  DIGIT_MASKS[0xc] = m(G)|m(E)|m(D);
  DIGIT_MASKS[0xd] = m(B)|m(G)|m(C)|m(D)|m(E);
  DIGIT_MASKS[0xE] = m(F)|m(E)|m(A)|m(G)|m(D);
  DIGIT_MASKS[0xF] = m(F)|m(E)|m(A)|m(G);
}

void init_digit_encoder() {
  init_digit_masks();
}

bool is_set(unsigned mask, int bit) {
  return mask & m(A);
}

char dispchar(unsigned mask, int bit) {
  return mask & m(bit) ? '*' : ' ';
}

void print_digit_mask(unsigned mask) {
  char display[7][7];

  sprintf(display[0], " %c%c%c ", dispchar(mask, A), dispchar(mask, A), dispchar(mask, A));
  sprintf(display[1], "%c   %c", dispchar(mask, F), dispchar(mask, B));
  sprintf(display[2], "%c   %c", dispchar(mask, F), dispchar(mask, B));
  sprintf(display[3], " %c%c%c ", dispchar(mask, G), dispchar(mask, G), dispchar(mask, G));
  sprintf(display[4], "%c   %c", dispchar(mask, E), dispchar(mask, C));
  sprintf(display[5], "%c   %c", dispchar(mask, E), dispchar(mask, C));
  sprintf(display[6], " %c%c%c  %c", dispchar(mask, D), dispchar(mask, D), dispchar(mask, D), dispchar(mask, DP));

  for (int i = 0; i < 7; ++i)
    printf("%s\n", display[i]);
}

void get_masks(const char *value, unsigned int digits[], int maxDigits) {

  int digitIndex = -1;

  for (char *pVal = (char *)value; *pVal != 0; ++pVal) {
    if (*pVal >= '0' && *pVal <= '9') {
      digits[++digitIndex] = DIGIT_MASKS[*pVal - '0'];
    }
    else if (*pVal >= 'a' && *pVal <= 'f') {
      digits[++digitIndex] = DIGIT_MASKS[0xa + (*pVal - 'a')];
    }
    else if (*pVal >= 'A' && *pVal <= 'F') {
      digits[++digitIndex] = DIGIT_MASKS[0xa + (*pVal - 'A')];
    }
    else if (*pVal == 'H') {
      digits[++digitIndex] = m(F)|m(B)|m(G)|m(C)|m(E);
    }
    else if (*pVal == 'M') {
      digits[++digitIndex] = m(G)|m(C)|m(E);
    }
    else if (*pVal == '_') {
      digits[++digitIndex] = m(D);
    }
    else if (*pVal == '.') {
      digits[digitIndex] |= m(DP);
    }
    else {
      digits[++digitIndex] = m(D); // Error mask.
    }
  }

  while (++digitIndex < maxDigits)
    digits[digitIndex] = 0;
}
