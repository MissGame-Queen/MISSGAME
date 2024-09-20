let arrData = JSON.parse(variable.get("value"));
let str = "{}";
let jsonReturn = {};
str = variableTable.name["GV_魔龍參數"]?.get("value") ?? "{}";
if (str === "") str = "{}";
jsonReturn = JSON.parse(str);

if (arrData.hasOwnProperty("playerDodge")) {
    if (jsonReturn.hasOwnProperty("魔龍累計擊中"))
        jsonReturn["魔龍累計擊中"] = parseInt(jsonReturn["魔龍累計擊中"]) + arrData.playerDodge.hit
    else
        jsonReturn["魔龍累計擊中"] = arrData.playerDodge.hit;

    if (arrData.playerDodge.miss > 0) {
        let json = {
            "value": 2//arrData.playerDodge.miss
        }
        variableTable.name["GV_魔龍閃躲出球"]?.save("value", JSON.stringify(json));
        if (jsonReturn.hasOwnProperty("魔龍累計閃躲")) {
            jsonReturn["魔龍累計閃躲"] = parseInt(jsonReturn["魔龍累計閃躲"]) + arrData.playerDodge.miss
        }
        else {
            jsonReturn["魔龍累計閃躲"] = arrData.playerDodge.miss;
        }
    }


}
if (arrData.hasOwnProperty("magicStar")) {
    if (jsonReturn.hasOwnProperty("魔龍累計星星"))
        jsonReturn["魔龍累計星星"] = parseInt(jsonReturn["魔龍累計星星"]) + arrData.magicStar
    else
        jsonReturn["魔龍累計星星"] = arrData.magicStar;
}
if (arrData.hasOwnProperty("ballRequest")) {
    if (jsonReturn.hasOwnProperty("魔龍累計BOUNS"))
        jsonReturn["魔龍累計BOUNS"] = parseInt(jsonReturn["魔龍累計BOUNS"]) + arrData.ballRequest
    else
        jsonReturn["魔龍累計BOUNS"] = arrData.ballRequest;
}
if (arrData.hasOwnProperty("nowHealth")) {
    jsonReturn["魔龍目前血量"] = arrData.nowHealth;
}



variableTable.name["GV_魔龍參數"]?.save("value", JSON.stringify(jsonReturn));

variableTable.name["GV_魔龍累計擊中"]?.save("value", String(jsonReturn["魔龍累計擊中"] ?? 0));
variableTable.name["GV_魔龍累計閃躲"]?.save("value", String(jsonReturn["魔龍累計閃躲"] ?? 0));
variableTable.name["GV_魔龍累計星星"]?.save("value", String(jsonReturn["魔龍累計星星"] ?? 0));
variableTable.name["GV_魔龍累計BOUNS"]?.save("value", String(jsonReturn["魔龍累計BOUNS"] ?? 0));
variableTable.name["GV_魔龍累計秒數"]?.save("value", String(jsonReturn["魔龍累計秒數"] ?? 0));
variableTable.name["GV_魔龍目前血量"]?.save("value", String(jsonReturn["魔龍目前血量"] ?? 0));

