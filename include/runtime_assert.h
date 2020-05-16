#include <printf.h>
#include <startup.h>

#define WARN_IF_NOT(x) if (!(x)) { CPU_SR_ALLOC(); OS_CRITICAL_ENTER(); printf("Warning: in %s:%d: `!(%s)'", __FILE__, __LINE__, #x); OS_CRITICAL_EXIT(); }
#define WARN_IF_NOT_OP(x, op, y) if (!(x op y)) { CPU_SR_ALLOC(); OS_CRITICAL_ENTER(); printf("Warning: in %s:%d: `!(%s %s %s)' (%s = %llu, %s = %llu)\n", __FILE__, __LINE__, #x, #op, #y, #x, x, #y, y); OS_CRITICAL_EXIT(); }
#define ASSERT(x) if (!(x)) { __asm__("cpsid if"); printf("ASSERTION FAILED in %s:%d: `%s'", __FILE__, __LINE__, #x); hang(); }
#define ASSERT_OP(x, op, y) if (!(x op y)) { __asm__("cpsid if"); printf("ASSERTION FAILED IN %s:%d: `%s %s %s' (%s = %llu, %s = %llu)\n", __FILE__, __LINE__, #x, #op, #y, #x, x, #y, y); hang(); }
