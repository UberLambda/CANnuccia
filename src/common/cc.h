#ifndef CC_H
#define CC_H

/// Marks that a function will never return.
#define CN_NORETURN __attribute__((noreturn))

/// Makes a function/variable reside in section `sec`.
#define CN_SECTION(sec) __attribute__((section(sec)))

#endif // CC_H
