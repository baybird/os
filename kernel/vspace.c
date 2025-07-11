#include <cdefs.h>
#include <defs.h>
#include <elf.h>
#include <memlayout.h>
#include <vspace.h>
#include <proc.h>
#include <x86_64.h>
#include <x86_64vm.h>

// given a virtual address and the vregion struct
// returns the phsical page index
static int
va2vpi_idx(struct vregion *r, uint64_t va)
{
  if (r->dir == VRDIR_UP)
    return (va - r->va_base) >> PAGE_SHIFT;
  else if (r->dir == VRDIR_DOWN)
    return (r->va_base - 1 - va) >> PAGE_SHIFT;
  else
    panic("va2vpi_idx: invalid direction");
}

// given a reference to a vpage_info struct
// returns its permissions with respect to the
// user bit, present bit, and writable bit
static int
x86perms(struct vpage_info *vpi)
{
  int perms = PTE_U; // always user if in a virtual region
  if (vpi->present)
    perms |= PTE_P;
  if (vpi->writable)
    perms |= PTE_W;
  return perms;
}

extern pml4e_t *kpml4;  // kernel page table

// allocates space for the kernel page table and populates
// it with the kernel's virtual address mapping after the
// virtual address space has been initialized by the kernel
void
vspacebootinit(void)
{
  kpml4 = setupkvm(); // sets up the kernel's page table
  vspaceinstallkern();  // installs the kernel mapping in the table
  seginit();   // segment table
}

// initializes a given vspace struct, by creating the page table
// setting the kernel part of the page table and then adding the
// appropriate regions
int
vspaceinit(struct vspace *vs)
{
  struct vregion *vr;
  if (!(vs->pgtbl = setupkvm()))
    return -1;

  for (vr = vs->regions; vr < &vs->regions[NREGIONS]; vr++) {
    memset(vr, 0, sizeof(struct vregion));
  }

  vs->regions[VR_CODE].dir   = VRDIR_UP;
  vs->regions[VR_HEAP].dir   = VRDIR_UP;
  vs->regions[VR_USTACK].dir = VRDIR_DOWN;

  return 0;
}

// Allocates physical memory for the virtual address range [from_va, from_va + sz),
// updates the bookkeeping structures tracked by vregion, does not make changes to 
// the architecture dependent page table. If `sz <= 0` then this function does nothing.
//
// NOTE: this function does NOT modify `vr->size` (caller is responsible for
// updating `vr->size` if necessary.) Also pay attention to the PGROUNDUP(from_va) 
// in the physical memory allocation loop.
//
int
vregionaddmap(struct vregion *vr, uint64_t from_va, uint64_t sz, short present, short writable)
{
  char *mem;
  uint64_t a;
  struct vpage_info *vpi;

  if (sz + from_va >= KERNBASE)
    return -1;
  if (sz <= 0)
    return 0;

  for (a = PGROUNDUP(from_va); a < from_va + sz; a += PGSIZE) {
    if (!(vpi = va2vpage_info(vr, a)))
      goto addmap_failure;

    // You can remove this check if you want to allow vregionaddmap to remap already
    // mapped pages, but many students typically *don't* intend to do this so we panic
    // for now.
    assert(!vpi->used);

    mem = kalloc();
    if (!mem)
      goto addmap_failure;
    memset(mem, 0, PGSIZE);

    vpi->used = 1;
    vpi->present = present;
    vpi->writable = writable;
    vpi->ppn = PGNUM(V2P(mem));
  }
  return sz;

addmap_failure:
  for (a -= PGSIZE; a >= PGROUNDUP(from_va); a -= PGSIZE) {
    assertm(vpi = va2vpage_info(vr, a), "vpi info missing");
    kfree(P2V(vpi->ppn << PT_SHIFT));

    vpi->used = 0;
    vpi->present = 0;
    vpi->writable = 0;
    vpi->ppn = 0;
  }
  return -1;
}


// Adds a mapping into the vregion at va of size sz with the given permissions and then
// copies the data present in data to these addresses
static int
vradddata(struct vregion *r, uint64_t va, char *data, int sz, short present, short writable)
{
  int ret;
  uint64_t i, n;
  struct vpage_info *vpi;

  if ((ret = vregionaddmap(r, va, sz, present, writable)) < 0)
    return ret;

  for (i = 0; i < sz; i += PGSIZE) {
    vpi = va2vpage_info(r, va + i);
    assert(vpi->used);
    n = min((uint64_t)sz - i, (uint64_t)PGSIZE);
    memmove(P2V(vpi->ppn << PT_SHIFT), data + i, n);
  }
  return 0;
}

// va must be page aligned
static int
vrloaddata(struct vregion *r, uint64_t va, struct inode *ip, uint offset, uint sz)
{
  uint i, n;
  struct vpage_info *vpi;
  assertm(va % PGSIZE == 0, "va must be page aligned");

  for (i = 0; i < sz; i += PGSIZE) {
    vpi = va2vpage_info(r, va + i);
    assertm(vpi->used, "page must be allocated");
    n = min(sz - i, (uint) PGSIZE);
    if (readi(ip, P2V(vpi->ppn << PT_SHIFT), offset + i, n) != n)
      return -1;
  }

  return 0;
}

// Initializes the code region in the given vspace and copies the
// code in init to the region. Also allocates space for the stack
// region of 1 page.
// Note: This should only be called by the initial process.
void
vspaceinitcode(struct vspace *vs, char *init, uint64_t size)
{
  // Place bottom of stack at address SZ_2G matching layout described in
  // memory.md.
  const uint64_t stack = SZ_2G;

  // code pages and 5 pages reserved for sds
  vs->regions[VR_CODE].va_base = 0x10000;
  vs->regions[VR_CODE].size = PGROUNDUP(size) + 5 * PGSIZE;
  assertm(
    vradddata(&vs->regions[VR_CODE], 0x10000, init, size, VPI_PRESENT, VPI_WRITABLE) == 0,
    "failed to allocate init code data"
  );
  assert(
    vregionaddmap(&vs->regions[VR_CODE], vs->regions[VR_CODE].va_base + PGROUNDUP(size), 5 * PGSIZE, VPI_PRESENT, VPI_WRITABLE) >= 0 
  ); 

  // Add the stack.
  vs->regions[VR_USTACK].va_base = stack;
  const size_t ustack_size = PGSIZE;
  vs->regions[VR_USTACK].size = ustack_size;
  assert(
    vregionaddmap(&vs->regions[VR_USTACK], stack - ustack_size, ustack_size, VPI_PRESENT, VPI_WRITABLE) >= 0
  );

  vspaceupdate(vs);
}

// loads the code for the given program at 'path' into the
// vspace for a process. The program must be ELF compliant. The
// first instruction for the program is returned in the output
// parameter rip
int
vspaceloadcode(struct vspace *vs, char *path, uint64_t *rip)
{
  bool first_section = true;
  int off, sz, i;
  uint64_t code_end;
  struct inode *ip;
  struct proghdr ph;
  struct elfhdr elf;

  if((ip = namei(path)) == 0){
    return 0;
  }

  locki(ip);

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto elf_failure;
  if(elf.magic != ELF_MAGIC)
    goto elf_failure;

  code_end = 0;

  // Load program into memory.
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto elf_failure;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto elf_failure;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto elf_failure;

    if (first_section) {
      vs->regions[VR_CODE].va_base = PGROUNDDOWN(ph.vaddr);
      first_section = false;
    }

    // use readelf --sections --program-headers -W <executable> to view the ELF headers and the permissions
    if((sz = vregionaddmap(&vs->regions[VR_CODE], (uint64_t)ph.vaddr, ph.memsz, VPI_PRESENT, (ph.flags & ELF_PROG_FLAG_WRITE) ? VPI_WRITABLE : VPI_READONLY)) < 0)
     goto elf_failure;
    if(ph.vaddr % PGSIZE != 0)
      goto elf_failure;

    code_end = ph.vaddr + ph.memsz;

    if(vrloaddata(&vs->regions[VR_CODE], ph.vaddr, ip, ph.off, ph.filesz) < 0)
     goto elf_failure;
  }

  assert(!first_section);

  // Set code region
  vs->regions[VR_CODE].size = code_end - vs->regions[VR_CODE].va_base;

  // Place after code, leave 1 page in between
  vs->regions[VR_HEAP].va_base = PGROUNDUP(code_end) + PGSIZE;
  vs->regions[VR_HEAP].size = 0;

  unlocki(ip);
  irelease(ip);
  *rip = elf.entry;
  return sz;
elf_failure:
  if(ip) {
    unlocki(ip);
    irelease(ip);
  }

  return 0;
}

// Rebuilds the hardware-dependent page table in `vs` to reflect the mappings in `vs->regions`.
// Call this to "flush" changes made to `vs->regions` to the hardware-specific page table.
//
// NOTE: this does NOT install the vspace. To install an address space see `vspaceinstall`.
void
vspaceupdate(struct vspace *vs)
{
  uint i;
  struct vregion *vr;
  struct vpage_info *vpi;
  uint64_t start, end;

  // First free the user entries (not the pages they point to)
  for (i = 0; i <= PML4_INDEX(SZ_4G); i++) {
    if(vs->pgtbl[i] & PTE_P){
      pdpte_t *pdpt = P2V(PDPT_ADDR(vs->pgtbl[i]));
      freevm_pdpt(pdpt);
      vs->pgtbl[i] = 0;
    }
  }

  // Then rebuild the user virtual address space
  for (vr = vs->regions; vr < &vs->regions[NREGIONS]; vr++) {
    start = VRBOT(vr);
    end = VRTOP(vr);

    assert(start % PGSIZE == 0);

    for (; start < end; start += PGSIZE) {
      vpi = va2vpage_info(vr, start);

      // The code vregion can have unused vpages in the middle, we shouldn't
      // add any mappings for those.
      if (vpi->used) {
        mappages(vs->pgtbl, start >> PT_SHIFT, 1, vpi->ppn, x86perms(vpi), 0);
      }
    }
  }
}

// Marks the current user address as not present in the page directory
// for the passed vspace.
// user_va must be rounded down to the nearest page.
void vspacemarknotpresent(struct vspace *vspace, uint64_t user_va) {
  struct vregion *vr;
  struct vpage_info *vpi;
  pte_t *pte;

  // Grab arguments and ensure they are valid and exist.
  assert(user_va % PGSIZE == 0);
  assert(vspace);
  vr = va2vregion(vspace, user_va);
  assert(vr);
  vpi = va2vpage_info(vr, user_va);
  assert(vpi);
  if (vpi->present) {
    panic("Passed user_va had present vpi.\n");
  }

  // Zero out the page table entry so the page is signalled as not
  // present.
  pte = walkpml4(vspace->pgtbl, (char *)user_va, 0);
  if (pte) {
    *pte = 0;
  }
}

// Installs the process' page table/vspace on the current cpu. After this
// function returns all addresses on this CPU are mapped using p's vspace. 
//
// Panics if p is null, there is no kernel stack initialized, or if there is no
// page table initialized.
void
vspaceinstall(struct proc *p)
{
  if (!p)
    panic("mrinstall: null proc");
  if (!p->kstack)
    panic("mrinstall: null kstack");
  if (!p->vspace.pgtbl)
    panic("mrinstall: page table not initialized");

  pushcli();  // turn off interrupts
  mycpu()->ts.rsp0 = (uint64_t)p->kstack + KSTACKSIZE;
  lcr3(V2P(p->vspace.pgtbl));
  popcli();  // turns on interrupts
}

// installs the kernel's page table on the cpu
void
vspaceinstallkern(void)
{
  lcr3(V2P(kpml4));
}

// recrusively frees the page descriptor linked list
// calling kfree on each page
static void
free_page_desc_list(struct vpi_page *page)
{
  assert((uint64_t) page % PGSIZE == 0);

  if (!page)
    return;

  free_page_desc_list(page->next);
  kfree((char *)page);
}

// frees the given vpsace by freeing each page that
// the vspace is using and then frees the underlying page
// table
void
vspacefree(struct vspace *vs)
{
  struct vregion *vr;

  for (vr = &vs->regions[0]; vr < &vs->regions[NREGIONS]; vr++) {
    free_page_desc_list(vr->pages);
    memset(vr, 0, sizeof(struct vregion));
  }

  freevm(vs->pgtbl);
}

// returns the region that a given virtual address exists
// in for the given vspace. 0 is returned if there is no
// vregion found
struct vregion*
va2vregion(struct vspace *vs, uint64_t va)
{
  struct vregion *vr;

  for (vr = &vs->regions[0]; vr < &vs->regions[NREGIONS]; vr++) {
    if (vr->dir == VRDIR_UP) {
      if (va >= vr->va_base && va < vr->va_base + vr->size)
        return vr;
    } else {
      if (va >= vr->va_base - vr->size && va < vr->va_base)
        return vr;
    }
  }
  return 0;
}

// gets the vpage_info struct for the given virtual address va
// in the vregion
struct vpage_info*
va2vpage_info(struct vregion *vr, uint64_t va)
{
  int idx;
  struct vpi_page *info;

  if (!vr->pages) {
    vr->pages = (struct vpi_page *)kalloc();
    memset(vr->pages, 0, PGSIZE);
  }

  idx = va2vpi_idx(vr, va);
  info = vr->pages;

  while (idx >= VPIPPAGE) {
    assertm(info, "idx was out of bounds");
    if (!info->next) {
      info->next = (struct vpi_page *)kalloc();
      if (!info->next)
        return 0;
      memset(info->next, 0, PGSIZE);
    }
    info = info->next;
    idx -= VPIPPAGE;
  }

  return &info->infos[idx];
}

// Tests if a vregion has [va, va + size) mapped in it's virtual address space.
// when size == 0, check if va is in the region
int
vregioncontains(struct vregion *vr, uint64_t va, int size)
{
  // make sure it's backwards-compatible when size == 0
  if (size == 0 && va == VRTOP(vr)) return false;

  return va >= VRBOT(vr) && va + size <= VRTOP(vr);
}


// Tests if a vspace contains [va, va + size).
int
vspacecontains(struct vspace *vs, uint64_t va, int size)
{
  struct vregion *vr = va2vregion(vs, va);
  if (!vr)
    return -1;
  return vregioncontains(vr, va, size);
}


// recursively copies the vpi_page struct from src to dst
//
// return 0 on success, -1 if failed
static int
copy_vpi_page(struct vpi_page **dst, struct vpi_page *src)
{
  int i;
  char *data;
  struct vpage_info *srcvpi, *dstvpi;

  if (!src) {
    *dst = 0;
    return 0;
  }

  if (!(*dst = (struct vpi_page *)kalloc()))
    return -1;

  memset(*dst, 0, sizeof(struct vpi_page));

  for (i = 0; i < VPIPPAGE; i++) {
    srcvpi = &src->infos[i];
    dstvpi = &(*dst)->infos[i];
    if (srcvpi->used) {
      dstvpi->used = srcvpi->used;
      dstvpi->present = srcvpi->present;
      dstvpi->writable = srcvpi->writable;
      if (!(data = kalloc()))
        return -1;
      memmove(data, P2V(srcvpi->ppn << PT_SHIFT), PGSIZE);
      dstvpi->ppn = PGNUM(V2P(data));
    }
  }

  return copy_vpi_page(&(*dst)->next, src->next);
}

// copies the regions and pagesof the src vspace to dst
int
vspacecopy(struct vspace *dst, struct vspace *src)
{
  struct vregion *vr;

  memmove(dst->regions, src->regions, sizeof(struct vregion) * NREGIONS);

  for (vr = dst->regions; vr < &dst->regions[NREGIONS]; vr++)
    if (copy_vpi_page(&vr->pages, vr->pages) < 0)
      return -1;

  vspaceupdate(dst);

  return 0;
}


// Initializes the stack vregion in the user's address space for the
// given vspace beginning at start and growing down from that address.
int
vspaceinitstack(struct vspace *vs, uint64_t start)
{
  struct vregion *vr = &vs->regions[VR_USTACK];
  vr->va_base = start;
  const size_t ustack_size = PGSIZE;
  vr->size = ustack_size;

  // stack page
  if (vregionaddmap(vr, start - ustack_size, ustack_size, VPI_PRESENT, VPI_WRITABLE) < 0)
    return -1;

  return 0;
}

// writes sz amount of the data provided into the virtual address space for the user
// at va. In the user space corresponding to vs if the data at va accessed it will
// correspond to the data provided to this method.
//
// va must be greater than 0 and not above the kernel base
int
vspacewritetova(struct vspace *vs, uint64_t va, char *data, int sz)
{
  uint64_t end, wsz;
  struct vpage_info *vpi;
  struct vregion *vr;

  assertm(sz > 0, "sz less than or equal to 0");
  assertm(va + sz < KERNBASE, "went over kernel vm base");

  end = va + sz;
  while (va < end) {
    wsz = min((int)(PGROUNDUP(va) - va), sz);

    if (!(vr = va2vregion(vs, va)))
      return -1;

    vpi = va2vpage_info(vr, va);
    assert(vpi->used);

    if (!vpi->writable)
      return -1;

    memmove(P2V(vpi->ppn << PT_SHIFT) + (va % PGSIZE), data, wsz);

    va += wsz;
    data += wsz;
    sz -= wsz;
  }

  return 0;
}

// dumps the first 10 words in the stack starting
// from the base and moving down 8 bytes at at time.
void
vspacedumpstack(struct vspace *vs) {
  struct vregion *vr = &vs->regions[VR_USTACK];
  uint64_t data;
  struct vpage_info *vpi;
  uint64_t starting_va;
  uint64_t ending_va;
  int words = 10;

  starting_va = vr->va_base - sizeof(uint64_t);
  ending_va = max(vr->va_base - vr->size, vr->va_base - words * (sizeof(uint64_t)));
  vpi = va2vpage_info(vr, starting_va);

  cprintf("dumping stack: base=%p size=%d\n", vr->va_base, vr->size);

  for (uint64_t va = starting_va; va >= ending_va; va -= sizeof(uint64_t)) {
    uint64_t la = (uint64_t) P2V(vpi->ppn << PT_SHIFT) + (va % PGSIZE);
    memmove(&data, (void *) la, sizeof(uint64_t));
    cprintf("virtual address: %p data: %lx\n", va, data);
  }
}

// Dump entire code region.
void
vspacedumpcode(struct vspace*vs) {
  struct vregion *vr = &vs->regions[VR_CODE];
  uint64_t data;
  struct vpage_info *vpi;
  uint64_t starting_va;
  uint64_t ending_va;

  starting_va = vr->va_base;
  vpi = va2vpage_info(vr, starting_va);

  cprintf("dumping code: base=%p size=%d\n", vr->va_base, vr->size);
  int va = starting_va;
  while(vpi && vpi->used) {
    ending_va = va + PGSIZE;
    for (; va < ending_va; va += sizeof(uint64_t)) {
      uint64_t la = (uint64_t) P2V(vpi->ppn << PT_SHIFT) + (va % PGSIZE);
      memmove(&data, (void *) la, sizeof(uint64_t));
      cprintf("virtual address: %p data: %lx\n", va, data);
    }
    vpi = va2vpage_info(vr, va);
  }
}
