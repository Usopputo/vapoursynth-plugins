const unsigned char *w_cur = w_chunk_base;
unsigned char *output_base0 = output + ((x0+0)*nOutputPlanes + oi0*OP_BLOCK_SIZE)*sizeof(float);
unsigned char *output_base1 = output + ((x0+1)*nOutputPlanes + oi0*OP_BLOCK_SIZE)*sizeof(float);
unsigned char *output_base2 = output + ((x0+2)*nOutputPlanes + oi0*OP_BLOCK_SIZE)*sizeof(float);
unsigned char *output_base3 = output + ((x0+3)*nOutputPlanes + oi0*OP_BLOCK_SIZE)*sizeof(float);

vreg_t oreg00 = zero_vreg();
vreg_t oreg01 = zero_vreg();

vreg_t oreg10 = zero_vreg();
vreg_t oreg11 = zero_vreg();

vreg_t oreg20 = zero_vreg();
vreg_t oreg21 = zero_vreg();

vreg_t oreg30 = zero_vreg();
vreg_t oreg31 = zero_vreg();

for (int dposx=0; dposx<3; dposx++) {
    int dposx2 = dposx-1;
    int dposx2_le = dposx-1;
    int dposx2_re = dposx-1;

    if (x0 == 0 && dposx == 0) {
        dposx2_le = 0;
    }

    if (x0 == width-4 && dposx == 2) {
        dposx2_re = 0;
    }

    int off0 = ((dposy2*width + x0 + 0 + dposx2_le)*nInputPlanes+ii0*IP_BLOCK_SIZE)*sizeof(float);
    int off1 = ((dposy2*width + x0 + 1 + dposx2)*nInputPlanes+ii0*IP_BLOCK_SIZE)*sizeof(float);
    int off2 = ((dposy2*width + x0 + 2 + dposx2)*nInputPlanes+ii0*IP_BLOCK_SIZE)*sizeof(float);
    int off3 = ((dposy2*width + x0 + 3 + dposx2_re)*nInputPlanes+ii0*IP_BLOCK_SIZE)*sizeof(float);
    const unsigned char *input_cur_x0 = in + off0;
    const unsigned char *input_cur_x1 = in + off1;
    const unsigned char *input_cur_x2 = in + off2;
    const unsigned char *input_cur_x3 = in + off3;

#if (defined USE_SSE3)
    for (int ii1=0; ii1<IP_BLOCK_SIZE; ii1+=4) {
        __m128 i0 = _mm_load_ps((float*)input_cur_x0);
        __m128 i1 = _mm_load_ps((float*)input_cur_x1);
        __m128 i2 = _mm_load_ps((float*)input_cur_x2);
        __m128 i3 = _mm_load_ps((float*)input_cur_x3);
        __m128 ireg0, ireg1, ireg2, ireg3;

        input_cur_x0 += 16;
        input_cur_x1 += 16;
        input_cur_x2 += 16;
        input_cur_x3 += 16;

        __m128 w0, w1;

#define ACCUMULATE(II)                                          \
        w0 = _mm_load_ps((float*)(w_cur));      \
        w1 = _mm_load_ps((float*)(w_cur+16));                 \
                                                                \
        w_cur += 32;                                          \
                                                                \
        ireg0 = _mm_shuffle_ps(i0, i0, _MM_SHUFFLE(II,II,II,II));       \
        oreg00 = madd_vreg(w0, ireg0, oreg00);                  \
        oreg01 = madd_vreg(w1, ireg0, oreg01);                  \
                                                                \
        ireg1 = _mm_shuffle_ps(i1, i1, _MM_SHUFFLE(II,II,II,II));       \
        oreg10 = madd_vreg(w0, ireg1, oreg10);                          \
        oreg11 = madd_vreg(w1, ireg1, oreg11);                  \
                                                                \
        ireg2 = _mm_shuffle_ps(i2, i2, _MM_SHUFFLE(II,II,II,II));        \
        oreg20 = madd_vreg(w0, ireg2, oreg20);                  \
        oreg21 = madd_vreg(w1, ireg2, oreg21);                  \
                                                                \
        ireg3 = _mm_shuffle_ps(i3, i3, _MM_SHUFFLE(II,II,II,II));        \
        oreg30 = madd_vreg(w0, ireg3, oreg30);                  \
        oreg31 = madd_vreg(w1, ireg3, oreg31);

        ACCUMULATE(0);
        ACCUMULATE(1);
        ACCUMULATE(2);
        ACCUMULATE(3);
    }

#elif (defined __ARM_NEON)

    for (int ii1=0; ii1<IP_BLOCK_SIZE; ii1+=16) {
        /* q0-q3: ireg
         * q4,q5 : wreg
         */
#define NEON_BODY(PLD_IN,PLD_W)                                         \
        __asm__ __volatile__("vld1.32 {q0}, [%[PTR0]:64]!\n\t"          \
                             "vld1.32 {q1}, [%[PTR1]:64]!\n\t"          \
                             "vld1.32 {q2}, [%[PTR2]:64]!\n\t"          \
                             "vld1.32 {q3}, [%[PTR3]:64]!\n\t"          \
                             "vld1.32 {q4}, [%[W_CUR]:64]!\n\t"         \
                             "vld1.32 {q5}, [%[W_CUR]:64]!\n\t"         \
                                                                        \
                             "vmla.f32 %q[OREG00], q4, d0[0]\n\t"       \
                             "vld1.32 {q6}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG01], q5, d0[0]\n\t"       \
                             "vld1.32 {q7}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG10], q4, d2[0]\n\t"       \
                             "vmla.f32 %q[OREG11], q5, d2[0]\n\t"       \
                             "vmla.f32 %q[OREG20], q4, d4[0]\n\t"       \
                             "vmla.f32 %q[OREG21], q5, d4[0]\n\t"       \
                             "vmla.f32 %q[OREG30], q4, d6[0]\n\t"       \
                             "vmla.f32 %q[OREG31], q5, d6[0]\n\t"       \
                                                                        \
                             "vmla.f32 %q[OREG00], q6, d0[1]\n\t"       \
                             "vld1.32 {q4}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG01], q7, d0[1]\n\t"       \
                             "vld1.32 {q5}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG10], q6, d2[1]\n\t"       \
                             "vmla.f32 %q[OREG11], q7, d2[1]\n\t"       \
                             "vmla.f32 %q[OREG20], q6, d4[1]\n\t"       \
                             "vmla.f32 %q[OREG21], q7, d4[1]\n\t"       \
                             "vmla.f32 %q[OREG30], q6, d6[1]\n\t"       \
                             "vmla.f32 %q[OREG31], q7, d6[1]\n\t"       \
                                                                        \
                             "vmla.f32 %q[OREG00], q4, d1[0]\n\t"       \
                             "vld1.32 {q6}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG01], q5, d1[0]\n\t"       \
                             "vld1.32 {q7}, [%[W_CUR]:64]!\n\t"         \
                             "vmla.f32 %q[OREG10], q4, d3[0]\n\t"       \
                             "vmla.f32 %q[OREG11], q5, d3[0]\n\t"       \
                             "vmla.f32 %q[OREG20], q4, d5[0]\n\t"       \
                             "vmla.f32 %q[OREG21], q5, d5[0]\n\t"       \
                             PLD_IN                                     \
                             "vmla.f32 %q[OREG30], q4, d7[0]\n\t"       \
                             "vmla.f32 %q[OREG31], q5, d7[0]\n\t"       \
                                                                        \
                             "vmla.f32 %q[OREG00], q6, d1[1]\n\t"       \
                             PLD_W                                      \
                             "vmla.f32 %q[OREG01], q7, d1[1]\n\t"       \
                             "vmla.f32 %q[OREG10], q6, d3[1]\n\t"       \
                             "vmla.f32 %q[OREG11], q7, d3[1]\n\t"       \
                             "vmla.f32 %q[OREG20], q6, d5[1]\n\t"       \
                             "vmla.f32 %q[OREG21], q7, d5[1]\n\t"       \
                             "vmla.f32 %q[OREG30], q6, d7[1]\n\t"       \
                             "vmla.f32 %q[OREG31], q7, d7[1]\n\t"       \
                                                                        \
                             :[W_CUR]"+r"(w_cur),                       \
                              [OREG00]"+w"(oreg00),                     \
                              [OREG01]"+w"(oreg01),                     \
                              [OREG10]"+w"(oreg10),                     \
                              [OREG11]"+w"(oreg11),                     \
                              [OREG20]"+w"(oreg20),                     \
                              [OREG21]"+w"(oreg21),                     \
                              [OREG30]"+w"(oreg30),                     \
                              [OREG31]"+w"(oreg31),                     \
                              [PTR0]"+r"(input_cur_x0),                 \
                              [PTR1]"+r"(input_cur_x1),                 \
                              [PTR2]"+r"(input_cur_x2),                 \
                              [PTR3]"+r"(input_cur_x3)                  \
                             :                                          \
                             :"q0","q1","q2","q3",                      \
                              "q4","q5","q6","q7","memory");

        NEON_BODY("pld [%[PTR0], #256]\n\t", "pld [%[W_CUR], #192]\n\t");
        NEON_BODY("","");
        NEON_BODY("","");
        NEON_BODY("","");
    }


#else

#define accumulate(o0,o1,w0,w1,addr) {                                  \
        vreg_t ireg0 = load_vreg_broadcast(addr);                       \
        o0 = madd_vreg(w0, ireg0, o0);                                  \
        o1 = madd_vreg(w1, ireg0, o1);                                  \
    }

#define MUL_W_IN(I)                                                     \
    {                                                                   \
        int I2 = I;                                                     \
        vreg_t wreg0, wreg1;                                            \
                                                                        \
        wreg0 = load_vreg(w_cur);                                       \
        wreg1 = load_vreg(w_cur + VEC_NELEM*sizeof(float));             \
                                                                        \
        accumulate(oreg00, oreg01, wreg0, wreg1, (input_cur_x0));       \
        accumulate(oreg10, oreg11, wreg0, wreg1, (input_cur_x1));       \
        accumulate(oreg20, oreg21, wreg0, wreg1, (input_cur_x2));       \
        accumulate(oreg30, oreg31, wreg0, wreg1, (input_cur_x3));       \
                                                                        \
        w_cur += OP_BLOCK_SIZE * sizeof(float);                         \
        input_cur_x0 += sizeof(float);                                  \
        input_cur_x1 += sizeof(float);                                  \
        input_cur_x2 += sizeof(float);                                  \
        input_cur_x3 += sizeof(float);                                  \
    }

    for (int ii1=0; ii1<IP_BLOCK_SIZE; ii1+=4) {
        MUL_W_IN(ii1+0);
        MUL_W_IN(ii1+1);
        MUL_W_IN(ii1+2);
        MUL_W_IN(ii1+3);
    }

#endif

}

if (dposy == 0 && ii0 == 0) {
    store_vreg(output_base0 + (        0)*sizeof(float), oreg00);
    store_vreg(output_base0 + (VEC_NELEM)*sizeof(float), oreg01);

    store_vreg(output_base1 + (        0)*sizeof(float), oreg10);
    store_vreg(output_base1 + (VEC_NELEM)*sizeof(float), oreg11);

    store_vreg(output_base2 + (        0)*sizeof(float), oreg20);
    store_vreg(output_base2 + (VEC_NELEM)*sizeof(float), oreg21);

    store_vreg(output_base3 + (        0)*sizeof(float), oreg30);
    store_vreg(output_base3 + (VEC_NELEM)*sizeof(float), oreg31);
} else if (last_iter) {
    vreg_t tmp00, tmp01;
    vreg_t tmp10, tmp11;
    vreg_t tmp20, tmp21;
    vreg_t tmp30, tmp31;

    vreg_t mtz, ltz, bv0, bv1;

    bv0 = load_vreg(biases + (oi0*OP_BLOCK_SIZE+        0)*sizeof(float));
    bv1 = load_vreg(biases + (oi0*OP_BLOCK_SIZE+VEC_NELEM)*sizeof(float));

#define ReLU(addr, bv, N)                               \
    tmp##N = load_vreg(addr);                           \
    tmp##N = add_vreg(tmp##N, oreg##N);                 \
    tmp##N = add_vreg(tmp##N, bv);                      \
    mtz = max_vreg(tmp##N, zero_vreg());                \
    ltz = min_vreg(tmp##N, zero_vreg());                \
    tmp##N = madd_vreg(ltz, set1_vreg(0.1f), mtz);      \

    ReLU(output_base0 + (        0)*sizeof(float), bv0, 00);
    ReLU(output_base0 + (VEC_NELEM)*sizeof(float), bv1, 01);
    ReLU(output_base1 + (        0)*sizeof(float), bv0, 10);
    ReLU(output_base1 + (VEC_NELEM)*sizeof(float), bv1, 11);
    ReLU(output_base2 + (        0)*sizeof(float), bv0, 20);
    ReLU(output_base2 + (VEC_NELEM)*sizeof(float), bv1, 21);
    ReLU(output_base3 + (        0)*sizeof(float), bv0, 30);
    ReLU(output_base3 + (VEC_NELEM)*sizeof(float), bv1, 31);

    store_vreg(output_base0 + (        0)*sizeof(float), tmp00);
    store_vreg(output_base0 + (VEC_NELEM)*sizeof(float), tmp01);
    store_vreg(output_base1 + (        0)*sizeof(float), tmp10);
    store_vreg(output_base1 + (VEC_NELEM)*sizeof(float), tmp11);
    store_vreg(output_base2 + (        0)*sizeof(float), tmp20);
    store_vreg(output_base2 + (VEC_NELEM)*sizeof(float), tmp21);
    store_vreg(output_base3 + (        0)*sizeof(float), tmp30);
    store_vreg(output_base3 + (VEC_NELEM)*sizeof(float), tmp31);
} else {
    vreg_t tmp;

#define ADD_TO_MEM(addr, val)                   \
    tmp = load_vreg(addr);                      \
    tmp = add_vreg(tmp, val);                   \
    store_vreg(addr, tmp);

    ADD_TO_MEM(output_base0 + (        0)*sizeof(float), oreg00);
    ADD_TO_MEM(output_base0 + (VEC_NELEM)*sizeof(float), oreg01);
    ADD_TO_MEM(output_base1 + (        0)*sizeof(float), oreg10);
    ADD_TO_MEM(output_base1 + (VEC_NELEM)*sizeof(float), oreg11);
    ADD_TO_MEM(output_base2 + (        0)*sizeof(float), oreg20);
    ADD_TO_MEM(output_base2 + (VEC_NELEM)*sizeof(float), oreg21);
    ADD_TO_MEM(output_base3 + (        0)*sizeof(float), oreg30);
    ADD_TO_MEM(output_base3 + (VEC_NELEM)*sizeof(float), oreg31);
}

#undef MUL_W_IN
#ifdef accumulate
#undef accumulate
#endif
#undef ADD_TO_MEM
#undef ReLU
