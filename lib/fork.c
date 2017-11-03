// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

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

	if (!(err & FEC_WR) || !(uvpd[PDX(addr)] & PTE_P) ||
	    !(uvpt[PGNUM(addr)] & PTE_COW))
		panic("pgfault libc err or cow missing");	
	
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	
        // LAB 4: Your code here.

	int perm = PTE_U | PTE_W | PTE_P;
	
	if ((r = sys_page_alloc(0, (void *)PFTEMP, perm)))
		panic("pgfault user: page alloc call");

	void *fault_page = (void *)ROUNDDOWN(addr, PGSIZE);
	memcpy((void *)PFTEMP, fault_page, PGSIZE);

	if ((r = sys_page_map(0, (void *)PFTEMP, 0, fault_page, perm)))
		panic("pgfault user: page map call");
	if ((r = sys_page_unmap(0, (void *)PFTEMP)))
		panic("pgfault user: page unmap of pftemp call");
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
	int r;

	// LAB 4: Your code here.

	int perm;
	void *page_va;
	pte_t pte;
	
	pte = uvpt[pn];
	perm = pte & 0xfff;
	page_va = (void *)(pn * PGSIZE);

	// first check for cow or write
	if (perm & (PTE_COW | PTE_W)) {
		perm = PTE_U | PTE_P | PTE_COW;
		//first create new page in envid
		if ((r = sys_page_map(0, page_va, envid, page_va, perm)))
			return r;
		//now recreate the page in current env
		if ((r = sys_page_map(0, page_va, 0, page_va, perm)))
			return r;
		return 0;
	}

	// not a writeable page, just copy it with old perms
	if ((r = sys_page_map(0, page_va, envid, page_va, perm)))
		return r;
	
	return 0;
}

// declared for use in fork()
extern void _pgfault_upcall();

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

	int r;
	envid_t child_id, parent;

	set_pgfault_handler(pgfault);

	child_id = sys_exofork();

	if (child_id == 0) {
		// in the child, so change thisenv
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}		

		
	// this is the parent env
	// first copy ALL userspace pages with duppage
	uintptr_t addr;
	for (addr = 0; addr < UXSTACKTOP - PGSIZE; addr += PGSIZE) {
		// check if pgtbl present via uvpd
		// then check pgtbl entry present and user
		if ( (uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P)
		     && (uvpt[PGNUM(addr)] & PTE_U))
			duppage(child_id, PGNUM(addr));
	}
	
	// now allocate a page for the child uxstack
	if ((r = sys_page_alloc(child_id, (void *)(UXSTACKTOP - PGSIZE),
				PTE_W|PTE_U|PTE_P)))
		return r;

	// now set the pgfault handler in child
	if ((r = sys_env_set_pgfault_upcall(child_id, _pgfault_upcall)))
		return r;

	// now mark as runnable and return childid to parent
	if ((r = sys_env_set_status(child_id, ENV_RUNNABLE)))
		return r;
	return child_id;
	
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
