#ifndef _PB_SYSTEM_VC_H_
#define PB_SYSTEM_VC_H_

#include <stdint.h>
#include <stddef.h>
#if defined(_MSC_VER) && (_MSC_VER >= 1800) // VisualStudio 2013 or above
#include <stdbool.h>
#else

#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined	1
#ifndef __cplusplus

// In VisualStudio 2010, compiler doesn't support C99, so we have to use "unsigned int" instead of "_Bool".
#define bool	unsigned int
#define false	0
#define true	1

#endif /* __cplusplus */
#endif /* __bool_true_false_are_defined */

#endif
#include <string.h>

#endif // PB_SYSTEM_VC_H_
