// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	if (!((err & FEC_WR) ||                  // Wasn't a write
	      (err & FEC_PR) ||                  // Addres not mapped
	      (uvpt[PGNUM(addr)] & PTE_COW))) {  // Page hasn't COW permision
		panic("Unable to handle page fault");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(sys_getenvid(), PFTEMP, PTE_U | PTE_W | PTE_P)) <
	    0) {
		panic("sys_page_alloc error on pgfault");
	}

	memmove(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);

	if ((r = sys_page_map(sys_getenvid(),
	                      PFTEMP,
	                      sys_getenvid(),
	                      ROUNDDOWN(addr, PGSIZE),
	                      PTE_U | PTE_W | PTE_P)) < 0) {
		panic("sys_page_map error on pgfault");
	}

	if (sys_page_unmap(0, (void *) PFTEMP) < 0)
		panic("sys_page_unmap error on pgfault");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.
	void *va = (void *) (pn * PGSIZE);
	int perm = uvpt[pn] & (PTE_COW | PTE_SYSCALL);

	if (!(perm & PTE_SHARE) && ((perm & PTE_W) || (perm & PTE_COW))) {
		perm &= ~PTE_W;
		perm = perm | PTE_COW;
	}

	if (sys_page_map(sys_getenvid(), va, envid, va, perm) < 0) {
		panic("first sys_page_map error on duppage");
	}
	if (sys_page_map(sys_getenvid(), va, sys_getenvid(), va, perm) < 0) {
		panic("second sys_page_map error on duppage");
	}

	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	if ((perm & PTE_W) == PTE_W) {
		if (sys_page_alloc(dstenv, va, perm) < 0)
			panic("sys_page_alloc error on dup or share");
		if (sys_page_map(dstenv, va, 0, UTEMP, perm) < 0)
			panic("sys_page_map error on dup or share");
		memmove(UTEMP, va, PGSIZE);
		if (sys_page_unmap(0, UTEMP) < 0)
			panic("sys_page_unmap error on dup or share");
	} else {
		if (sys_page_map(0, va, dstenv, va, perm) < 0)
			panic("sys_page_map error on dup or share");
	}
}

envid_t
fork_v0(void)
{
	envid_t envid = sys_exofork();
	uint8_t *addr = 0;

	if (envid < 0)
		panic("sys_exofork on fork_v0");
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	for (addr; addr < (uint8_t *) UTOP; addr += PGSIZE) {
		if ((uvpd[PDX(addr)] & PTE_P) != PTE_P) {
			continue;
		}
		int perm = uvpt[PGNUM(addr)];
		if ((perm & PTE_P) == PTE_P) {
			dup_or_share(envid, addr, perm & PTE_SYSCALL);
		}
	}
	int err = sys_env_set_status(envid, ENV_RUNNABLE);
	if (err < 0)
		panic("sys_env_set_status on fork_v0");
	return envid;
}


//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//

envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	extern void _pgfault_upcall(void);
	envid_t envid = sys_exofork();
	uint8_t *addr = 0;
	if (envid < 0)
		panic("sys_exofork on fork_v0");
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (addr; addr < (uint8_t *) UTOP; addr += PGSIZE) {
		if ((uvpd[PDX(addr)] & PTE_P) != PTE_P) {
			continue;
		}
		int perm = uvpt[PGNUM(addr)];
		if ((perm & PTE_P) == PTE_P) {
			duppage(envid, PGNUM(addr));
		}
	}

	if (sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE), PTE_SYSCALL) <
	    0) {
		panic("sys_env_set_status failed on fork");
	}
	if (sys_env_set_pgfault_upcall(envid, _pgfault_upcall) < 0) {
		panic("sys_env_set_status failed on fork");
	}
	if (sys_env_set_status(envid, ENV_RUNNABLE) < 0)
		panic("sys_env_set_status failed on fork");

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
