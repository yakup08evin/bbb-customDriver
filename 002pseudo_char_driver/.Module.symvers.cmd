cmd_/home/yakup/workSpaceDriver/ldd/custom_drivers/002pseudo_char_driver/Module.symvers := sed 's/ko$$/o/' /home/yakup/workSpaceDriver/ldd/custom_drivers/002pseudo_char_driver/modules.order | scripts/mod/modpost -m -a  -o /home/yakup/workSpaceDriver/ldd/custom_drivers/002pseudo_char_driver/Module.symvers -e -i Module.symvers   -T -
