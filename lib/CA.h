#include <vector>    // maybe?

/*
	TODO: retool
	0) make a class
	1) return a vector of vectors with all CA codes
	2) alternate invocation to just return a single CA

	so likely one function to generate one CA, with a loop to call for all 32

	3) code to generate 'early' and 'late'
*/


void
generate_prn_codes(void) {
  // This code was cribbed from Cliff Kelly's SoftOSGPS which is "freely distributed without license"
  int i, j, G1, G2, prn, chip, half_chip;
  int G2_i[33] = {0x000, 0x3f6, 0x3ec, 0x3d8, 0x3b0, 0x04b, 0x096, 0x2cb, 0x196,
                  0x32c, 0x3ba, 0x374, 0x1d0, 0x3a0, 0x340, 0x280, 0x100, 0x113,
                  0x226, 0x04c, 0x098, 0x130, 0x260, 0x267, 0x338, 0x270, 0x0e0,
                  0x1c0, 0x380, 0x22b, 0x056, 0x0ac, 0x158};
  for (prn = 1; prn < 33; prn++) {
    char prn_code[1023];
    prn_code[0] = 1;
    PRN[prn].push_back(1);
    G1 = 0x1FF;
    G2 = G2_i[prn];
    for (chip = 1; chip < 1023; chip++) {
      prn_code[chip] = (G1 ^ G2) & 0x1; // exor the right hand most bit
      PRN[prn].push_back(prn_code[chip]);
      i = ((G1 << 2) ^ (G1 << 9)) & 0x200;
      G1 = (G1 >> 1) | i;
      j = ((G2 << 1) ^ (G2 << 2) ^ (G2 << 5) ^ (G2 << 7) ^ (G2 << 8) ^
           (G2 << 9)) &
          0x200;
      G2 = (G2 >> 1) | j;
    }
  }
}
