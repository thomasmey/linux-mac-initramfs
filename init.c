#define _GNU_SOURCE         /* See feature_test_macros(7) */
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/loop.h>

static void setup_stdio(void) {
  int confd = open("/dev/console", O_RDWR); 
  dup2(confd, 0);
  dup2(confd, 1);
  dup2(confd, 2);
  close(confd);
}

int main(void) {

  const char* sysroot = "/sysroot";
  const char* sysdev = "/dev/sda2";

  const char* looproot = "/sysloop";
  const char* loopdev = "/dev/loop0";

  int rc;

  // establish stdin,stdout,stderr
  setup_stdio();

  // mount sys root
  rc = mount(sysdev, sysroot, "hfsplus", 0, "");
  if (rc == -1) {
	perror("Mount sysroot");
  }
  rc = chdir(sysroot);
  if (rc == -1) {
	perror("chdir sysroot");
  }

  int fd_root_img = open("/sysroot/fedora/rootfs.img", O_RDWR);
  if (fd_root_img == -1) {
	perror("open rootfs.img");
  }

  int fd_loop = open(loopdev, O_RDWR);
  if (fd_loop == -1) {
	perror("open loopdev");
  }
  rc = ioctl(fd_loop, LOOP_SET_FD, fd_root_img);
  if (rc == -1) {
	perror("ioctl loopdev");
  }
  rc = close(fd_loop);
  if (rc == -1) {
	perror("close loopdev");
  }

  // mount loop device
  rc = mount(loopdev, looproot, "ext4", 0, "");
  if (rc == -1) {
	perror("mount loopdev");
  }
  rc = chdir(looproot);
  if (rc == -1) {
	perror("chdir looproot");
  }

  // move to new root fs
  rc = mount(looproot, "/", NULL, MS_MOVE, NULL);
  if (rc == -1) {
	perror("move looproot");
  }

  rc = chroot(".");
  if (rc == -1) {
	perror("chroot");
  }
  // establish stdin,stdout,stderr
  setup_stdio();

  // call original init
  execl("/sbin/init", "/sbin/init", (char*) NULL);

  // unmount loop
  rc = umount(loopdev);
  if (rc == -1) {
	perror("umount loopdev");
  }

  fd_loop = open(loopdev, O_RDWR);
  if (fd_loop == -1) {
	perror("open loopdev");
  }
  rc = ioctl(fd_loop, LOOP_CLR_FD, fd_root_img);
  if (rc == -1) {
	perror("ioctl loopdev");
  }
  rc = close(fd_loop);
  if (rc == -1) {
	perror("close loopdev");
  }

  rc = close(fd_root_img);
  if (rc == -1) {
	perror("close root img");
  }

  rc = umount(sysroot);
  if (rc == -1) {
	perror("umount sysroot");
  }

}
