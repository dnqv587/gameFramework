--print("hello world");
logger.debug("hello world");

key = ""
function PrintTable(table , level)
    level = level or 1
    local indent = ""
    for i = 1, level do
      indent = indent.."  "
    end
   
    if key ~= "" then
      print(indent..key.." ".."=".." ".."{")
    else
      print(indent .. "{")
    end
   
    key = ""
    for k,v in pairs(table) do
       if type(v) == "table" then
          key = k
          PrintTable(v, level + 1)
       else
          local content = string.format("%s%s = %s", indent .. "  ",tostring(k), tostring(v))
        print(content)  
        end
    end
    print(indent .. "}")
   
  end
mysql.connect("127.0.0.1", 3306, "test", "root", "20171028", function(err,conn) 
    if(err) then
        print(err)
end
    logger.debug("connect success lua")

    mysql.query(conn, "select * from person", function(err,reuslt,data)

        if(err) then
            logger.error("query error")
            return
        end
        PrintTable(reuslt)
    
    
    end
    );
end);

redis.connect("127.0.0.1", 6379, function(err,conn,data)
    if err then
        logger.error("redis connect error")
        return
    end
    --[[
    redis.query(conn, "hmset 001001 name \"blake\" age \"32\"", function(err,ret,data)
    if(err) then
        log_error("redis query error")
        return
    end
    print(ret)
    
    end)
    ]]
    redis.query(conn, "hgetall 001001", function(err,ret,data)
        if(err) then
            log_error("redis query error")
            return
        end
        PrintTable(ret)

    end)

    redis.close(conn)
end,NULL);

