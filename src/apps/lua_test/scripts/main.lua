--初始化日志模块
logger.init("conf/log", "log", true);
--初始化协议模块
local proto_type={
    PROTO_JSON=0;
    PROTO_BUF=1;
}
proto_man.init(proto_type.PROTO_BUF);

--protobuf--注册映射表
if(proto_man.proto_type()==proto_type.PROTO_BUF) then
    


    local cmd_name_map=require("cmd_name_map")
        if cmd_name_map then 
            
        end
end

--开启网络服务
netbus.tcp_listen(8080);
netbus.ws_listen(6080);
netbus.udp_listen(9090);

logger.debug("start service")