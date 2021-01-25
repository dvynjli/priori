/*
 *	IB700 Single Board Computer WDT driver
 *
 *	2015: Modified by Markus Kusano. Added some driver code and adpated some of
 *	the functions to make the interpreter happy. Original copyright messages:
 *
 *	(c) Copyright 2001 Charles Howes <chowes@vsol.net>
 *
 *      Based on advantechwdt.c which is based on acquirewdt.c which
 *       is based on wdt.c.
 *
 *	(c) Copyright 2000-2001 Marek Michalkiewicz <marekm@linux.org.pl>
 *
 *	Based on acquirewdt.c which is based on wdt.c.
 *	Original copyright messages:
 *
 *	(c) Copyright 1996 Alan Cox <alan@redhat.com>, All Rights Reserved.
 *				http://www.redhat.com
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Neither Alan Cox nor CymruNet Ltd. admit liability nor provide
 *	warranty for any of this software. This material is provided
 *	"AS-IS" and at no charge.
 *
 *	(c) Copyright 1995    Alan Cox <alan@redhat.com>
 *
 *      14-Dec-2001 Matt Domsch <Matt_Domsch@dell.com>
 *           Added nowayout module option to override CONFIG_WATCHDOG_NOWAYOUT
 *           Added timeout module option to override default
 *
 */
#include "../verify.h"

using namespace std;

atomic<int> ibwdt_is_open;
// markus: originally this was a `spinlock_t` but I've changed it to simply be
// an int and provided by own simple spin lock implementation.
atomic<int> ibwdt_lock;
atomic<int> expect_close;

#define PFX "ib700wdt: "

/*
 *
 * Watchdog Timer Configuration
 *
 * The function of the watchdog timer is to reset the system
 * automatically and is defined at I/O port 0443H.  To enable the
 * watchdog timer and allow the system to reset, write I/O port 0443H.
 * To disable the timer, write I/O port 0441H for the system to stop the
 * watchdog function.  The timer has a tolerance of 20% for its
 * intervals.
 *
 * The following describes how the timer should be programmed.
 *
 * Enabling Watchdog:
 * MOV AX,000FH (Choose the values from 0 to F)
 * MOV DX,0443H
 * OUT DX,AX
 *
 * Disabling Watchdog:
 * MOV AX,000FH (Any value is fine.)
 * MOV DX,0441H
 * OUT DX,AX
 *
 * Watchdog timer control table:
 * Level   Value  Time/sec | Level Value Time/sec
 *   1       F       0     |   9     7      16
 *   2       E       2     |   10    6      18
 *   3       D       4     |   11    5      20
 *   4       C       6     |   12    4      22
 *   5       B       8     |   13    3      24
 *   6       A       10    |   14    2      26
 *   7       9       12    |   15    1      28
 *   8       8       14    |   16    0      30
 *
 */

// div: not used

int wd_times[] = {
	30,	/* 0x0 */
	28,	/* 0x1 */
	26,	/* 0x2 */
	24,	/* 0x3 */
	22,	/* 0x4 */
	20,	/* 0x5 */
	18,	/* 0x6 */
	16,	/* 0x7 */
	14,	/* 0x8 */
	12,	/* 0x9 */
	10,	/* 0xA */
	8,	/* 0xB */
	6,	/* 0xC */
	4,	/* 0xD */
	2,	/* 0xE */
	0,	/* 0xF */
};


#define WDT_STOP 0x441
#define WDT_START 0x443

/* Default timeout */
#define WD_TIMO 0		/* 30 seconds +/- 20%, from table */

atomic<int> wd_margin;


// markus: Manually handling the kernel parameters default initialization
#define WATCHDOG_NOWAYOUT 0

// markus: these kernel functions are ignored
#define module_param(v1, v2, v3)
//  do { } while (false)
#define MODULE_PARM_DESC(a, b)

atomic<int> nowayout;
// module_param(nowayout, int, 0);
// MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");


// Markus: Simulated buffer from userspace for idwdt_write()
atomic<int> write_user_buf;

// Simulated hardware port for WDT_START
// div: changing from char to atomic int
atomic<int> wdt_start_port;

// Simulated hardware port for WDT_STOP
// div: changing from char to atomic int
atomic<int> wdt_stop_port;

/*
 *	Kernel methods.
 */
// div: useless definitions start
// markus: the prink() function is assumed to be benign and ignored 
#define printk(a)

// markus: some stuff used in printk
#define KERN_INFO
//#define PFX
// markus: error number which i've taken the liberty to define
#define EIO 29

// markus: kernel defined function
#define spin_lock_init(lk) do { lk = false; } while(false)

// markus: kernel defined function
// it should return non_det but for now we consider it just to return true
#define misc_register(a) 0

// Markus: again, this always succeedds. Should be non-det
#define request_region(a, b, c) 0

// markus: again, this always suceeds but should probably register an "event
// hander"
#define register_reboot_notifier(a) 0

// markus: should be implemented
#define release_region(a, b)  do { } while (false) 
#define misc_deregister(a)  do { } while (false)
// div: useless definitions end

// Markus: this is a kernel defined function
// Our simulated version simply takes the first parameter and assigns it to the
// second
#define outb_p(v, l) \
  do { \
    l = v; \
  } while (false)

// Markus: this is a kernel defined function
// It is slightly simplified since it is ignoring the weak-memory semantics of
// the architecture. I've also simplified it even more to not be bitwise
#define clear_bit(v, l) \
  do { \
    l = v; \
  } while (false)

// Markus: Spin lock implementation. This was originially provided by the
// kernel
#define spin_lock(lk) \
  do { \
    while (lk) { \
      /* spin */ \
    } \
  } while (false)

// Markus: parallel to spin_lock()
#define spin_unlock(lk) \
  do { \
    lk = 0; \
  } while (false)

// Markus: forcing this to be inlined via a maco. In the original program, this
// wrote wd_margin to port WDT_START
#define idwdt_ping() \
  do { \
    outb_p(wd_margin, wdt_start_port); \
  } while (false)

// Markus: this file originally took a `struct file` and `loff_t`. But, it did
// not use them. They have been removed to simplify things. Aditionally, it
// took a pointer from userspace to a buffer. To realize this pointer, it has
// been moved to the global space (see write_user_buf). It has also been
// simplified to a char instead of a char*.
// Aditionally, even though LLVM supports inlineing, I've old-school macroed
// this to force it to be inlined 
//
// Count is assumed to be either zero or non-zero indicating that the value of
// write_user_buf contains valid data (count != zero) or if it should not be
// read (count == zero)
#define ibwdt_write(count) \
  do { \
    if (count) { \
		  if (!nowayout) { \
        /* In case it was set long ago */ \
        expect_close = 0; \
        /* Markus: Originially, the buffer would be looped over here */ \
        if (write_user_buf == 'V') { \
          expect_close = 42; \
        } \
      } \
    } \
    idwdt_ping(); \
  } while (false)

// markus: again, this was originally a real function but I've macroed it to
// force it to be inlined. The assert statement was not orginally in the code
// but was rather a critical printk message
//
// This originally had two arguments but they were unused
#define ibwdt_close() \
  do { \
    spin_lock(ibwdt_lock); \
    if (expect_close == 42) { \
      /* markus: write of zero to WDT_STOP port */\
		  outb_p(0, wdt_stop_port); \
    } \
    else { \
      /* markus: original error message modified to assertion */ \
      /* printk(KERN_CRIT PFX "WDT device closed unexpectedly.  WDT will not stop!\n"); */ \
      assert(0); \
    } \
    clear_bit(0, ibwdt_is_open); \
    expect_close = 0; \
    spin_unlock(ibwdt_lock); \
  } while (false)

// div: useless definitions start
// markus: again, this function was originall a real function which i've
// macroed to inline
#define ibwdt_init() \
  do { \
	  int res; \
	printk(KERN_INFO PFX "WDT driver for IB700 single board computer initialising.\n"); \
	spin_lock_init(ibwdt_lock); \
	res = misc_register(&ibwdt_miscdev); \
	if (res) { \
		printk (KERN_ERR PFX "failed to register misc device\n"); \
		goto out_nomisc; \
	} \
    /* markus: originally, the following macro was here, but it will always
     * evaluate to true in our case
     */ \
/* #if WDT_START != WDT_STOP */ \
	if (!request_region(WDT_STOP, 1, "IB700 WDT")) { \
		printk (KERN_ERR PFX "STOP method I/O %X is not available.\n"); \
		res = -EIO; \
		goto out_nostopreg; \
	} \
/* markus: see note above about this parameter */ \
/* #endif */ \
	if (!request_region(WDT_START, 1, "IB700 WDT")) { \
		printk (KERN_ERR PFX "START method I/O %X is not available.\n"); \
		res = -EIO; \
		goto out_nostartreg; \
	} \
	res = register_reboot_notifier(&ibwdt_notifier); \
	if (res) { \
		printk (KERN_ERR PFX "Failed to register reboot notifier.\n"); \
		goto out_noreboot; \
	} \
  /* markus: we cannot return anything */ \
	/* return 0; */ \
out_noreboot:  \
	release_region(WDT_START, 1); \
out_nostartreg: \
/* markus: again, this will always evaluate true */ \
/* #if WDT_START != WDT_STOP */ \
	release_region(WDT_STOP, 1); \
/* #endif */ \
out_nostopreg: \
	misc_deregister(&ibwdt_miscdev); \
out_nomisc: \
	/* return res; */ \
  } while (false)


// markus: these kernel functions are ignored
#define module_init(a)
#define module_exit(a)
#define MODULE_AUTHOR(a)
#define MODULE_DESCRIPTION(a)
#define MODULE_LICENSE(a)
#define MODULE_ALIAS_MISCDEV(a)
// markus: defined by kernel, ignored by us (tm)
#define WATCHDOG_MINOR
module_init(ibwdt_init);
module_exit(ibwdt_exit);

MODULE_AUTHOR("Charles Howes <chowes@vsol.net>");
MODULE_DESCRIPTION("IB700 SBC watchdog driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
// div: useless definitions end

/* end of ib700wdt.c */

// markus: driver thread for the character device driver. 
void *writer(void *unused) {
  // div: inlining the function calls
  // ibwdt_write(0);
  // div: the loop will run for only one iteration, removing.
  // do {
    if (0) {
    	  int rnowayout = nowayout.load(ACQ);
		  if (!rnowayout) { 
        /* In case it was set long ago */ 
        expect_close.store(0,REL); 
        /* Markus: Originially, the buffer would be looped over here */ 
        int wbuf = write_user_buf.load(ACQ);
        if (wbuf == 'V') { 
          expect_close.store(42, REL); 
        } 
      } 
    } 
    // div: inlining function call
    // idwdt_ping(); 
    // div: the loop will run for only one iteration, removing.
    // do {
      // div: inline
      // outb_p(wd_margin, wdt_start_port);
      int r_wd_margin = wd_margin.load(ACQ);
      wdt_start_port.store(r_wd_margin, REL);
    // } while (false)
  // } while (false)
  return NULL;
}

// markus: driver thread for the character device driver. 
void *closer(void *unused) {
  // Tell the driver we expect to close it
  write_user_buf.store('V', REL);
  
  // div: inlining function call
  // ibwdt_write(1);
  // div: the loop will run for only one iteration, removing.
  // do {
    if (1) {
    	  int rnowayout = nowayout.load(ACQ);
		  if (!rnowayout) { 
        /* In case it was set long ago */ 
        expect_close.store(0,REL); 
        /* Markus: Originially, the buffer would be looped over here */ 
        int wbuf = write_user_buf.load(ACQ);
        if (wbuf == 'V') { 
          expect_close.store(42, REL); 
        } 
      } 
    } 
  
  // Close the driver
  // div: inlining the function call
  // ibwdt_close();
  // div: the loop will run for only one iteration, removing.
  // do {
  	// div: inlining spin lock
    // spin_lock(ibwdt_lock);
    // div: the loop will run for only one iteration, removing.
    // do { \
      // since iwdt_lock is false, replacing it with simple load
      // while (ibwdt_lock) {
      ibwdt_lock.load(ACQ);
        /* spin */
      // }
    // } while (false)
    int r_expect_close = expect_close.load(ACQ);
    if (r_expect_close == 42) {
      /* markus: write of zero to WDT_STOP port */
      	  // div: inlining function call
		  // outb_p(0, wdt_stop_port);
		  wdt_stop_port.store(0, REL);
    }
    else {
      /* markus: original error message modified to assertion */
      /* printk(KERN_CRIT PFX "WDT device closed unexpectedly.  WDT will not stop!\n"); */
      assert(0);
    }
    // div: inlining function call
    // clear_bit(0, ibwdt_is_open);
    ibwdt_is_open.store(0, REL);
    expect_close.store(0, REL);
    // div: inlining function call
    // spin_unlock(ibwdt_lock);
    // div: the loop will run for only one iteration, removing.
    // do {
      // lk = 0;
      ibwdt_lock.store(0, REL);
    // } while (false)
  // } while (false)
  
  return NULL;
}

int main(int argc, char *argv[]) {
  // initialization
  nowayout.store(WATCHDOG_NOWAYOUT, REL);
  expect_close.store(0, REL);
  ibwdt_lock.store(0, REL);
  wd_margin.store(WD_TIMO, REL);
  pthread_t t1;
  pthread_t t2;
  pthread_create(&t1, NULL, writer, NULL);
  pthread_create(&t2, NULL, closer, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  return 0;
}

