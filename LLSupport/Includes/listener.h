extern void add_listener (LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm);
extern void remove_listeners (LIST * notifiees);
extern void delete_listener (LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm);
extern void notify_listeners (LIST * notifiees);
