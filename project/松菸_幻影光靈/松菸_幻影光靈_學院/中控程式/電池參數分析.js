/**
 * {"battery":3.95,"id":60}
 */
let str = variableTable.name["GV_電池封包資料"]?.get("value") ?? "";
let strTEST="";
let jsonBattery = {};
let arr=[];
try {
    if (str !== "") {

        jsonBattery = JSON.parse(str);
        str = variableTable.name["GV_武器電量_整理前"]?.get("value") ?? "[]";
        if (str !== "") {
             arr = JSON.parse(str);
            let ishave = false;
            arr.forEach(element => {
                if (element.id === jsonBattery.id) {
                    element.battery = jsonBattery.battery;
                    ishave = true;
                }
            });
            if (ishave === false) {
                
                arr.push(jsonBattery);
            }
        }
        //strTEST+=JSON.stringify(arr);
        variableTable.name["GV_武器電量_整理前"]?.save("value", JSON.stringify(arr));

        let strOUT="";
        arr.forEach(element => {
            strOUT+=`ID:${element.id}的電池電壓為${element.battery}V\n`
        });
        variableTable.name["GV_武器電量"]?.save("value", strOUT);
    
    }
} catch (error) {

}
//variableTable.name["GV_TEST"]?.save("value", strTEST);
