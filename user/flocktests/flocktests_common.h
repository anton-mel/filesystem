#pragma once

#define TMO   2000   /* ms to wait before declaring a hang                */
#define RETRY 10000  /* iterations in stress/fuzzer loops                 */


#define FLOCK_SH  (1U << 0)   /* shared/read            */
#define FLOCK_EX  (1U << 1)   /* exclusive/write        */
#define FLOCK_UN  (1U << 2)   /* unlock                 */
#define FLOCK_NB  (1U << 3)   /* non-blocking acquire   */

#define FLOCK_EWOULDBLOCK  (-2)
#define FLOCK_TEST_PATH "tmp.tat"

#define R_EXP_NB 7
#define R_EXP_B 8
#define W_EXP_NB 9
#define W_EXP_B 10

#define W_DOES_B 11