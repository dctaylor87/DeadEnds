extern void add_listener (List **notifiees, CALLBACK_FNC fncptr, void* uparm);
extern void remove_listeners (List **notifiees);
extern void delete_listener (List **notifiees, CALLBACK_FNC fncptr, void* uparm);
extern void notify_listeners (List **notifiees);
