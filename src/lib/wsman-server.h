
struct __WsManListenerH {
       /* Plugins */
       GList *plugins;	
};
typedef struct __WsManListenerH WsManListenerH;

WsContextH wsman_init_plugins(WsManListenerH *listener);
