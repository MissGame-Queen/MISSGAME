
let str = "{}";
let jsonReturn = {};
str = variableTable.name["GV_魔龍參數"]?.get("value") ?? "{}";
jsonReturn = JSON.parse(str);

const keys = Object.keys(jsonReturn);

keys.forEach(key => {
    if (key === "魔龍累計擊中")
        jsonReturn[key] = variableTable.name["GV_魔龍累計擊中"].get("value");
    else if (key === "魔龍累計閃躲")
        jsonReturn[key] = variableTable.name["GV_魔龍累計閃躲"].get("value");
    else if (key === "魔龍累計星星")
        jsonReturn[key] = variableTable.name["GV_魔龍累計星星"].get("value");
    else if (key === "魔龍累計BOUNS")
        jsonReturn[key] = variableTable.name["GV_魔龍累計BOUNS"].get("value");
    else if (key === "魔龍累計秒數")
        jsonReturn[key] = variableTable.name["GV_魔龍累計秒數"].get("value");
    else if (key === "魔龍目前血量")
        jsonReturn[key] = variableTable.name["GV_魔龍目前血量"].get("value");
});
let Time = parseInt(variableTable.name["GV_魔龍累計秒數"]?.get("value"));
if (Time > 0)
    jsonReturn["魔龍累計秒數"] = Time;

variableTable.name["GV_魔龍參數"]?.save("value", JSON.stringify(jsonReturn));
