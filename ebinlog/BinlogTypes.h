#ifndef __MY_TYPES_H
#define __MY_TYPES_H

#define MYSQL_TYPE_DECIMAL          0
#define MYSQL_TYPE_TINY             1          
#define MYSQL_TYPE_SHORT            2
#define MYSQL_TYPE_LONG             3
#define MYSQL_TYPE_FLOAT            4
#define MYSQL_TYPE_DOUBLE           5
#define MYSQL_TYPE_NULL             6
#define MYSQL_TYPE_TIMESTAMP        7
#define MYSQL_TYPE_LONGLONG         8
#define MYSQL_TYPE_INT24            9
#define MYSQL_TYPE_DATE             10
#define MYSQL_TYPE_TIME             11
#define MYSQL_TYPE_DATETIME         12
#define MYSQL_TYPE_YEAR             13
#define MYSQL_TYPE_NEWDATE          14
#define MYSQL_TYPE_VARCHAR          15
#define MYSQL_TYPE_BIT              16
#define MYSQL_TYPE_STRING           254
#define MYSQL_TYPE_NEWDECIMAL       246
#define MYSQL_TYPE_ENUM             247
#define MYSQL_TYPE_SET              248
#define MYSQL_TYPE_TINY_BLOB        249
#define MYSQL_TYPE_MEDIUM_BLOB      250
#define MYSQL_TYPE_LONG_BLOB        251
#define MYSQL_TYPE_BLOB             252
#define MYSQL_TYPE_VAR_STRING       253
#define MYSQL_TYPE_GEOMETRY         255

#define ROTATE_EVENT                4
#define TABLE_MAP_EVENT             19
#define WRITE_ROWS_EVENT            23
#define UPDATE_ROWS_EVENT           24
#define DELETE_ROWS_EVENT           25

#define DEBUG(format,args...) printf(format,##args)

#endif

#ifndef __MY_CONVERT_H
#define __MY_CONVERT_H

#include <stdlib.h>
#include <stdio.h>

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t	    int32;
typedef int64_t     int64;

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

#define sint2korr(A)    (*((int16 *) (A)))
#define sint3korr(A)    ((int32) ((((uint8) (A)[2]) & 128) ?    \
                        (((uint32) 255L << 24) |                \
                        (((uint32) (uint8) (A)[2]) << 16) |     \
                        (((uint32) (uint8) (A)[1]) << 8) |      \
                        ((uint32) (uint8) (A)[0])) :            \
                        (((uint32) (uint8) (A)[2]) << 16) |     \
                        (((uint32) (uint8) (A)[1]) << 8) |      \
                        ((uint32) (uint8) (A)[0])))
#define sint4korr(A)    (*((int32 *) (A)))
#define uint8korr(A)    ((uint64)(((uint32) ((uint8) (A)[0])) + \
                        (((uint32) ((uint8) (A)[1])) << 8) +    \
                        (((uint32) ((uint8) (A)[2])) << 16) +   \
                        (((uint32) ((uint8) (A)[3])) << 24)) +  \
                        (((uint64) (((uint32) ((uint8) (A)[4]))+\
                        (((uint32) ((uint8) (A)[5])) << 8) +    \
                        (((uint32) ((uint8) (A)[6])) << 16) +   \
                        (((uint32) ((uint8) (A)[7])) << 24))) <<\
                        32))
            
#define sint8korr(A)    (int64) uint8korr(A)
#define uint2korr(A)    (*((uint16*) (A)))
#define uint3korr(A)    (uint32) (((uint32) ((uint8) (A)[0])) + \
                        (((uint32) ((uint8) (A)[1])) << 8) +    \
                        (((uint32) ((uint8) (A)[2])) << 16))
#define uint4korr(A)    (*((uint32 *) (A)))
#define uint6korr(A)    ((int64)(((uint32) ((uint8) (A)[0]))+   \
                        (((uint32) ((uint8) (A)[1])) << 8)   +  \
                        (((uint32) ((uint8) (A)[2])) << 16)  +  \
                        (((uint32) ((uint8) (A)[3])) << 24)) +  \
                        (((int64) ((uint8) (A)[4])) << 32) +    \
                        (((int64) ((uint8) (A)[5])) << 40))

#define float4get(V,M)   do { *((float *) &(V)) = *((float*) (M)); } while(0)

typedef union {
  double v;
  long m[2];
} doubleget_union;

#define doubleget(V,M)              \
do { doubleget_union _tmp;          \
     _tmp.m[0] = *((long*)(M));     \
     _tmp.m[1] = *(((long*) (M))+1);\
     (V) = _tmp.v; } while(0)

#define float8get(V,M)   doubleget((V),(M))

int DecodePacketInt(const char *pBuf,uint32 *pPos) {
    int n = (int)(uint8)(*pBuf); 
    int i = 0;
    if (n < 251) {
        *pPos += 1;
        return n; 
    }
    else if (n == 251) {
        *pPos += 1;
        return 0;
    }
    else if (n == 252) {
        i = uint2korr((pBuf + 1)); 
        *pPos += 3;
        return i;  
    }
    else if (n == 253) {
        i = uint3korr((pBuf + 1)); 
        *pPos += 4;
        return i;
    }                   
    else {               
        i = uint4korr((pBuf + 1)); 
        *pPos += 9;
        return i;
    }
} 

#endif  

#ifndef __MY_DECIMAL_H
#define __MY_DECIMAL_H

#define DIG_PER_DEC1        9
#define E_DEC_OK            0
#define E_DEC_TRUNCATED     1
#define E_DEC_OVERFLOW      2
#define E_DEC_FATAL_ERROR   30
#define DECIMAL_BUFF_LENGTH 9

#define swap_variables(t, a, b) { t dummy; dummy= a; a= b; b= dummy; }
#define UNINIT_VAR(x) x= x

#define FIX_INTG_FRAC_ERROR(len, intg1, frac1, error)                   \
        do                                                              \
        {                                                               \
          if (unlikely(intg1+frac1 > (len)))                            \
          {                                                             \
            if (unlikely(intg1 > (len)))                                \
            {                                                           \
              intg1=(len);                                              \
              frac1=0;                                                  \
              error=E_DEC_OVERFLOW;                                     \
            }                                                           \
            else                                                        \
            {                                                           \
              frac1=(len)-intg1;                                        \
              error=E_DEC_TRUNCATED;                                    \
            }                                                           \
          }                                                             \
          else                                                          \
            error=E_DEC_OK;                                             \
        } while(0)


#define __builtin_expect(x, expected_value) (x)
#define unlikely(x) __builtin_expect((x),0)

#define DIG_BASE     1000000000
#define DIG_MAX      (DIG_BASE-1)

#define decimal_make_zero(dec)        do {                  \
                                        (dec)->buf[0]=0;    \
                                        (dec)->intg=1;      \
                                        (dec)->frac=0;      \
                                        (dec)->sign=0;      \
                                      } while(0)
#define E_DEC_BAD_NUM       8
#define ROUND_UP(X)  (((X)+DIG_PER_DEC1-1)/DIG_PER_DEC1)
#define mi_sint1korr(A) ((int8)(*A))
#define mi_sint2korr(A) ((int16)    (((int16) (((uint8*) (A))[1])) + ((int16) ((int16) ((char*) (A))[0]) << 8)))
#define mi_sint3korr(A) ((int32)    (((((uint8*) (A))[0]) & 128) ?          \
                                    (((uint32) 255L << 24) |                \
                                    (((uint32) ((uint8*) (A))[0]) << 16) |  \
                                    (((uint32) ((uint8*) (A))[1]) << 8) |   \
                                    ((uint32) ((uint8*) (A))[2])) :         \
                                    (((uint32) ((uint8*) (A))[0]) << 16) |  \
                                    (((uint32) ((uint8*) (A))[1]) << 8) |   \
                                    ((uint32) ((uint8*) (A))[2])))
#define mi_sint4korr(A) ((int32)    (((int32) (((uint8*) (A))[3])) +        \
                                    ((int32) (((uint8*) (A))[2]) << 8) +    \
                                    ((int32) (((uint8*) (A))[1]) << 16) +   \
                                    ((int32) ((int16) ((char*) (A))[0]) << 24)))

typedef int32 decimal_digit_t;
typedef decimal_digit_t dec1;

static const int dig2bytes[DIG_PER_DEC1+1]={0, 1, 1, 2, 2, 3, 3, 4, 4, 4};
static const dec1 powers10[DIG_PER_DEC1+1]={
  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

inline int decimal_operation_results(int result) {
  return result;
}
int decimal_bin_size(int precision, int scale) {
  int intg=precision-scale,
      intg0=intg/DIG_PER_DEC1, frac0=scale/DIG_PER_DEC1,
      intg0x=intg-intg0*DIG_PER_DEC1, frac0x=scale-frac0*DIG_PER_DEC1;

  return intg0*sizeof(dec1)+dig2bytes[intg0x]+
         frac0*sizeof(dec1)+dig2bytes[frac0x];
}

inline int my_decimal_get_binary_size(uint precision, uint scale) {
  return decimal_bin_size((int)precision, (int)scale);
}

inline int check_result(uint mask, int result) {
  if (result & mask)
    decimal_operation_results(result);
  return result;
}

typedef struct st_decimal_t {
  int    intg, frac, len;
  bool sign;
  decimal_digit_t *buf;
} decimal_t;

class my_decimal :public decimal_t
{
  decimal_digit_t buffer[DECIMAL_BUFF_LENGTH];

public:

  void init()
  {
    len= DECIMAL_BUFF_LENGTH;
    buf= buffer;
#if !defined (HAVE_purify) && !defined(DBUG_OFF)
    /* Set buffer to 'random' value to find wrong buffer usage */
    for (uint i= 0; i < DECIMAL_BUFF_LENGTH; i++)
      buffer[i]= i;
#endif
  }
  my_decimal()
  {
    init();
  }
  void fix_buffer_pointer() { buf= buffer; }

  bool sign() const { return decimal_t::sign; }
  void sign(bool s) { decimal_t::sign= s; }
  uint precision() const { return intg + frac; }

  /** Swap two my_decimal values */
  void swap(my_decimal &rhs)
  {
    swap_variables(my_decimal, *this, rhs);
    /* Swap the buffer pointers back */
    swap_variables(decimal_digit_t *, buf, rhs.buf);
  }
};


#define my_alloca(A) my_malloc((A),MYF(0))
#define my_afree(A) my_free((A))

int bin2decimal(const unsigned char *from, decimal_t *to, int precision, int scale)
{
  int error=E_DEC_OK, intg=precision-scale,
      intg0=intg/DIG_PER_DEC1, frac0=scale/DIG_PER_DEC1,
      intg0x=intg-intg0*DIG_PER_DEC1, frac0x=scale-frac0*DIG_PER_DEC1,
      intg1=intg0+(intg0x>0), frac1=frac0+(frac0x>0);
  dec1 *buf=to->buf, mask=(*from & 0x80) ? 0 : -1;
  const unsigned char *stop;
  unsigned char *d_copy;
  int bin_size= decimal_bin_size(precision, scale);

//  sanity(to);
  d_copy= (unsigned char*) malloc(bin_size);
  memset(d_copy,0,bin_size);
  memcpy(d_copy, from, bin_size);
  d_copy[0]^= 0x80;
  from= d_copy;

  FIX_INTG_FRAC_ERROR(to->len, intg1, frac1, error);
  if (unlikely(error))
  {
    if (intg1 < intg0+(intg0x>0))
    {
      from+=dig2bytes[intg0x]+sizeof(dec1)*(intg0-intg1);
      frac0=frac0x=intg0x=0;
      intg0=intg1;
    }
    else
    {
      frac0x=0;
      frac0=frac1;
    }
  }

  to->sign=(mask != 0);
  to->intg=intg0*DIG_PER_DEC1+intg0x;
  to->frac=frac0*DIG_PER_DEC1+frac0x;

  if (intg0x)
  {
    int i=dig2bytes[intg0x];
    dec1 UNINIT_VAR(x);
    switch (i)
    {
      case 1: x=mi_sint1korr(from); break;
      case 2: x=mi_sint2korr(from); break;
      case 3: x=mi_sint3korr(from); break;
      case 4: x=mi_sint4korr(from); break;
    }
    from+=i;
    *buf=x ^ mask;
    if (((uint64)*buf) >= (uint64) powers10[intg0x+1])
      goto err;
    if (buf > to->buf || *buf != 0)
      buf++;
    else
      to->intg-=intg0x;
  }
  for (stop=from+intg0*sizeof(dec1); from < stop; from+=sizeof(dec1))
  {
    *buf=mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    if (buf > to->buf || *buf != 0)
      buf++;
    else
      to->intg-=DIG_PER_DEC1;
  }
  for (stop=from+frac0*sizeof(dec1); from < stop; from+=sizeof(dec1))
  {
    *buf=mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    buf++;
  }
  if (frac0x)
  {
    int i=dig2bytes[frac0x];
    dec1 UNINIT_VAR(x);
    switch (i)
    {
      case 1: x=mi_sint1korr(from); break;
      case 2: x=mi_sint2korr(from); break;
      case 3: x=mi_sint3korr(from); break;
      case 4: x=mi_sint4korr(from); break;
    }
    *buf=(x ^ mask) * powers10[DIG_PER_DEC1 - frac0x];
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    buf++;
  }
  free(d_copy);
  return error;

err:
  free(d_copy);
  decimal_make_zero(((decimal_t*) to));
  return(E_DEC_BAD_NUM);
}

inline int binary2my_decimal(uint8 mask, const unsigned char *bin, my_decimal *d, int prec, int scale)
{
  return check_result(mask, bin2decimal(bin, (decimal_t*) d, prec, scale));
}

#endif
