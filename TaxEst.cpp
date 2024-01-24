#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>

const char *SW_VERSION =    "1.02";
const char *SW_DATE =       "2024-01-24";

#define NUM_BRACKETS        (7)
typedef enum
{
    FILING_STATUS_SINGLE    =   0
    , FILING_STATUS_MFJ     =   1
    , NUM_FILING_STATUS     =   2
}   filingStatus_t;

typedef struct
{
    double      rate;
    uint32_t    lowerLimit;
    uint32_t    upperLimit;
}   bracketElement_t;

bracketElement_t    taxBracket_SINGLE_2023[NUM_BRACKETS] =
{
    {.10,   0,        11000},
    {.12,   11001,    44725},
    {.22,   44726,    95375},
    {.24,   95376,    182100},
    {.32,   182101,   231250},
    {.35,   231251,   578125},
    {.37,   578126,   999999}
};

bracketElement_t    taxBracket_MFJ_2023[NUM_BRACKETS] =
{
    {.10,   0,         22000},
    {.12,   22001,     89450},
    {.22,   89451,     190750},
    {.24,   190751,    364200},
    {.32,   364201,    462500},
    {.35,   462501,    693750},
    {.37,   693751,    999999}
};

bracketElement_t    taxBracket_SINGLE_2024[NUM_BRACKETS] =
{
    {.10,   0,        11600},
    {.12,   11601,    47150},
    {.22,   47151,    100525},
    {.24,   100526,   191950},
    {.32,   191951,   243725},
    {.35,   243726,   609350},
    {.37,   690351,   999999}
};

bracketElement_t    taxBracket_MFJ_2024[NUM_BRACKETS] =
{
    {.10,   0,         23200},
    {.12,   23201,     94300},
    {.22,   94301,     201050},
    {.24,   201051,    383900},
    {.32,   383901,    487450},
    {.35,   487451,    731200},
    {.37,   731201,    999999}
};

void usage(const char *prog, const char *extraLine = (const char *)(NULL));

void usage(const char *prog, const char *extraLine)
{
    fprintf(stderr, "%s Ver %s %s\n", prog, SW_VERSION, SW_DATE);
    fprintf(stderr, "usage: %s <options>\n", prog);
    fprintf(stderr, "-f filingStatus           0 = SINGLE, 1 = MFJ\n");
    fprintf(stderr, "-y taxYear                2023 or 2024\n");
    fprintf(stderr, "-i (or -t) taxable income\n");
    fprintf(stderr, "-g gross income           enter if you want effective tax rate calculation.\n");
    fprintf(stderr, "-q quiet                  print tax only\n");
    if (extraLine) fprintf(stderr, "\n%s\n", extraLine);
}

int main(int argc, char *argv[])
{
    int                 opt;
    double              tax = 0.0;
    bool                usageError = false;
    bool                quietFlag = false;
    bracketElement_t    *pTaxBracket = (bracketElement_t *)(NULL);
    filingStatus_t      filingStatus = NUM_FILING_STATUS;
    uint32_t            taxYear = 0;
    uint32_t            taxableIncome = 0;
    uint32_t            grossIncome = 0;
    uint32_t            temp;

    struct option longOptions[] =
    {
        {"filingStatus",    required_argument,  0,      'f'}
        ,{"year",           required_argument,  0,      'y'}
        ,{"income",         required_argument,  0,      'i'}
        ,{"taxable",        required_argument,  0,      't'}
        ,{"gross",          required_argument,  0,      'g'}
        ,{"quiet",          no_argument,        0,      'q'}
        ,{0,0,0,0}
    };

    while (1)
    {
        int optionIndex = 0;
        opt = getopt_long(argc, argv, "f:y:i:t:g:qh?", longOptions, &optionIndex);

        if (-1 == opt) break;

        switch (opt)
        {
        case 'f':
            temp = strtoul(optarg, NULL, 10);
            if (temp >= NUM_FILING_STATUS)
            {
                usageError = true;
            }
            else
            {
                filingStatus = (filingStatus_t)(temp);
            }
            break;
        case 'y':
            taxYear = strtoul(optarg, NULL, 10);
            if  (   (taxYear < 2023)
                 || (taxYear > 2024)
                )
            {
                usageError = true;
            }
            break;
        case 'i':
        case 't':
            taxableIncome = strtoul(optarg, NULL, 10);
            if  (   (taxableIncome == 0)
                 || (taxableIncome > 999999)
                )
            {
                usageError = true;
            }
            break;
        case 'g':
            grossIncome = strtoul(optarg, NULL, 10);
            break;
        case 'q':
            quietFlag = true;
            break;
        case 'h':
        case '?':
        default:
            usageError = true;
            break;
        }
    }

    // ------------ TEST CODE ---------
    #if 0
    filingStatus = FILING_STATUS_SINGLE;
    taxYear = 2023;
    income = 77276;
    usageError = false;
    // ----------- END OF TEST CODE
    #endif

    if (usageError)
    {
        usage(basename(argv[0]));
        return -1;
    }

    switch (taxYear)
    {
        case 2023:
            pTaxBracket = (FILING_STATUS_SINGLE == filingStatus) ? taxBracket_SINGLE_2023 : taxBracket_MFJ_2023;
            break;
        case 2024:
            pTaxBracket = (FILING_STATUS_SINGLE == filingStatus) ? taxBracket_SINGLE_2024 : taxBracket_MFJ_2024;
            break;
        default:
            usage(basename(argv[0]));
            return -1;
            break;
    }

    while (taxableIncome > pTaxBracket->upperLimit)
    {
        tax += (double)(pTaxBracket->upperLimit - pTaxBracket->lowerLimit) * pTaxBracket->rate;
        pTaxBracket++;
    }
    tax += (double)(taxableIncome - pTaxBracket->lowerLimit) * pTaxBracket->rate;

    if (false == quietFlag)
    {
        printf("Tax Year      : %d\n", taxYear);
        printf("Filing Status : %s\n"
            ,(FILING_STATUS_SINGLE == filingStatus) ? "Single" : "MFJ"
            );
        printf("Taxable Income: $%d\n", taxableIncome);
        printf("Marginal Rate : %.0lf%%\n", pTaxBracket->rate * 100.0);
        if (grossIncome > 0)
        {
            printf("Gross Income  : $%d\n", grossIncome);
            printf("Effective Rate: %0.2lf%%\n", tax / (double)grossIncome * 100.0);
        }
        printf("\n *** Tax ***  : $%.2lf\n", tax);
    }
    else
    {
        printf("Tax = $%.2lf\n", tax);
    }

    return 0;
}
