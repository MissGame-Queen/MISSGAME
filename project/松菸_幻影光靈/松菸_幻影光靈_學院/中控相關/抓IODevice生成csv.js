const Parse = require('parse/node');

/**
 * 初始化 Parse Server
 */
Parse.initialize(
    "NumLockEscapeServerIsASystemForEscapeRoomDesignID",
    "",
    "NumLockEscapeServerIsASystemForEscapeRoomDesignMaster"
);
Parse.serverURL = "http://192.168.0.8:3000/parse";
// IODevice, AutoIO, Trigger, 

/**
 * 
 * IODeivce > name : string > set
 * AutoIO > ioNames : array > map
 * Trigger > conditionStr : string > replce
 */
const fs = require('fs');

// 读取 CSV 文件内容
const csvData = fs.readFileSync('example.csv', 'utf8');

// 将 CSV 数据按行分割成数组
const rows = csvData.replace(/\r\n/g, '\n').trim().split('\n');

// 将每一行再按逗号分割成数组，形成二维数组
//var arrIODevice = rows.map(row => row.split(','));
const eCSV = {
    'name': 0,
    'link': 1,
};
//console.log(arrIODevice);
var csvString = '\uFEFF';
var arrIODevice = [];
var arrTrigger = [];
var arrAutoIO = [];
/**
 * 匹配csv中的link並套用對應
 */
new Parse.Query("IODevice").limit(200).find().then(list => {
    // return;
    arrIODevice.push(['name', 'link', 'IODevice']);
    list.reverse().forEach(item => {
        let link = item.get("link");
        let name = item.get("name");
        let arr = [];
        if (link !== name) {
            arr.push(name);
            arr.push(link);
            arrIODevice.push(arr);
        }
    });
    csvString += (arrIODevice.map(row => row.join(',')).join('\n')) + '\n';
    //console.log(csvString);
    return new Parse.Query("Trigger").limit(200).find();
})
    .then(list => {

        arrTrigger.push(['conditionStr', 'link', 'Trigger']);
        list.reverse().forEach(item => {
            let conditionStr = item.get("conditionStr");
            let arr = String(conditionStr).match(/"IODevice":"([^"]+)"/g) ?? "";
            if (arr !== "") {
                arr.map(item => {
                    let str = item.match(/"IODevice":"([^"]+)"/)[1];
                    //console.log(str);
                    //let arr=[str];
                    arrTrigger.push(str);
                }
                )
            }
        });
        // 将数组转换为 Set 对象，去除重复的元素
        const uniqueItems = new Set(arrTrigger);
        // 将 Set 对象转换回数组
        arrTrigger = Array.from(uniqueItems);
        for (let i = 1; i < arrTrigger.length; i++) {
            let link;
            let name;
            let arr = [];
            for (let j = 1; j <= arrIODevice.length; j++) {
                if (j === arrIODevice.length) {
                    arr.push(arrTrigger[i]);
                    arr.push("");
                    break;
                }
                name = arrIODevice[j][0];
                if (name === arrTrigger[i]) {
                    link = arrIODevice[j][1];
                    arr.push(name);
                    arr.push(link);
                    break;
                }
            }
            arrTrigger[i] = [arr];
        }
        //console.log(arrTrigger);
        csvString += (arrTrigger.map(row => row.join(',')).join('\n'))+'\n';
        return new Parse.Query("AutoIO").limit(200).find();
    })
    .then(list => {

        arrAutoIO.push(['ioName', 'link', 'AutoIO']);
        list.reverse().forEach(item => {
            let ioName = item.get("ioName") ?? "";
            if (ioName !== "")
                ioName.forEach(item => {
                    arrAutoIO.push(item);
                })
        });

        // 将数组转换为 Set 对象，去除重复的元素
        const uniqueItems = new Set(arrAutoIO);
        // 将 Set 对象转换回数组
        arrAutoIO = Array.from(uniqueItems);
        for (let i = 1; i < arrAutoIO.length; i++) {
            let link;
            let name;
            let arr = [];
            for (let j = 1; j <= arrIODevice.length; j++) {
                if (j === arrIODevice.length) {
                    arr.push(arrAutoIO[i]);
                    arr.push("");
                    break;
                }
                name = arrIODevice[j][0];
                if (name === arrAutoIO[i]) {
                    link = arrIODevice[j][1];
                    arr.push(name);
                    arr.push(link);
                    break;
                }
            }
            arrAutoIO[i] = arr;

        }
        csvString += (arrAutoIO.map(row => row.join(',')).join('\n'))+'\n';
        // 将数据写回到 example.csv 文件中
        //?記得加'\uFEFF'不然Excel打開會是亂碼
        fs.writeFileSync('IO紀錄表.csv', csvString, 'utf8');
        //return new Parse.Query("AutoIO").limit(200).find();
    })
