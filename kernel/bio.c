// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#define NBUCKETS 13
#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock[NBUCKETS];
  struct spinlock glock;
  struct buf buf[NBUF];
  struct buf hashbucket[NBUCKETS]; 
} bcache;

void
binit(void)
{
  initlock(&bcache.glock,"glock");
  for(int i = 0; i< NBUCKETS;i++){
    char name[7];
    snprintf(name,sizeof(name),"bcahce_%d",i);
    initlock(&bcache.lock[i],name);
  }
  
  for(int i=0;i<NBUCKETS;i++){
    struct buf *tbuf=&bcache.hashbucket[i];
    tbuf->prev=tbuf;
    tbuf->next=tbuf;
    for(int j=0;j<NBUF;j++){
      if(j%NBUCKETS==i){
        bcache.buf[j].next = tbuf->next;
        bcache.buf[j].prev = tbuf;
        initsleeplock(&bcache.buf[j].lock, "buffer");
        tbuf->next->prev = &bcache.buf[j];
        tbuf->next = &bcache.buf[j]; 
      }
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  struct buf *tbuf;

  int id=blockno%NBUCKETS;

  acquire(&bcache.lock[id]);

  // Is the block already cached?
  tbuf =&bcache.hashbucket[id];
  for(b = tbuf->next; b != tbuf; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
	release(&bcache.lock[id]);

  //拿到缓存的全局锁，使得只有一个cpu能访问
  acquire(&bcache.glock);
	acquire(&bcache.lock[id]);
  tbuf =&bcache.hashbucket[id];
	//可能出现在进入全局锁之前，有另一个cpu的同个buf未命中而阻塞
	//此时再获得进入后，应该先对原bucket进行检查，是不是已经被存入了
  for(b = tbuf->next; b != tbuf; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[id]);
      release(&bcache.glock);
      acquiresleep(&b->lock);
      return b;
    }
  }
	//先对自身所在哈希桶的链表进行检查，减少换桶的情况，提高效率
	for(b = tbuf->prev;b!=tbuf;b = b->prev){
		if(b->refcnt == 0){
			b->dev=dev;
			b->blockno=blockno;
			b->valid=0;
			b->refcnt=1;
			release(&bcache.lock[id]);
			release(&bcache.glock);
			acquiresleep(&b->lock);
			return b; 
		}
	}

	//对其他12个哈希桶进行遍历
  struct buf *obuf;
  for(int i=1;i<NBUCKETS;i++){
    int otherid = (id+i)%NBUCKETS;

    acquire(&bcache.lock[otherid]);
    obuf = &bcache.hashbucket[otherid];
    for(b = obuf->prev;b!=obuf;b = b->prev){
      if(b->refcnt == 0){
				//更改链表链接
    	  b->prev->next=b->next;
        b->next->prev=b->prev;
				//更新数据
        b->dev=dev;
        b->blockno=blockno;
        b->valid=0;
        b->refcnt=1;
        release(&bcache.lock[otherid]);
				//插入到原bucket的链表中
        tbuf =&bcache.hashbucket[id];
        b->next=tbuf->next;
        b->prev=tbuf;
        tbuf->next->prev=b;
        tbuf->next=b;
        release(&bcache.lock[id]);
        
        release(&bcache.glock);
        acquiresleep(&b->lock);
        return b; 
      }
    }
    release(&bcache.lock[otherid]);
  }        
  release(&bcache.lock[id]);
  release(&bcache.glock);
  panic("bget: no buffers");

  
  
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int id=b->blockno%NBUCKETS;

  acquire(&bcache.lock[id]);
  struct buf *tbuf=&bcache.hashbucket[id];

  b->refcnt--;

  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = tbuf->next;
    b->prev = tbuf;
    tbuf->next->prev = b;
    tbuf->next = b;
  }
  
  release(&bcache.lock[id]);
}

void
bpin(struct buf *b) {
  int id = b->blockno%NBUCKETS;
  acquire(&bcache.lock[id]);
  b->refcnt++;
  release(&bcache.lock[id]);
}

void
bunpin(struct buf *b) {
  int id = b->blockno%NBUCKETS;
  acquire(&bcache.lock[id]);
  b->refcnt--;
  release(&bcache.lock[id]);
}


