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
var csvArray = rows.map(row => row.split(','));
const eCSV = {
    'type': 0,
    'link': 1,
    'name': 2,
    'old': 3,
    'new': 4,
    'raw': 5,
};
//console.log(csvArray);
/**
 * 匹配csv中的link並套用對應
 */
new Parse.Query("IODevice").limit(200).find().then(list => {
    //return new Parse.Query("Trigger").limit(200).find();
    let arrInput = [];
    let arrOutput = [];
    //修正後
    arrInput = [];
    arrOutput = [];
    list.reverse().forEach(item => {
        let link = item.get("link");
        let name = item.get("name");
        let isfind=false;
        for (let index = 0; index < csvArray.length; index++) {
            //console.log(link,",",csvArray[index][eCSV.link]);            
            if (link === csvArray[index][eCSV.link]&&name!==csvArray[index][eCSV.name]) {
                //item.save('name', csvArray[index][eCSV.new]);
                 name = item.get("name");
                csvArray[index][eCSV.name] = name;
                //console.log(link, ",", name);
                isfind=true;
                break;
            }
        }
        if(!isfind){
            name = item.get("name");
                csvArray[csvArray.length][eCSV.name] = name;
        }
    });
    const csvString = csvArray.map(row => row.join(',')).join('\n');
    // 将数据写回到 example.csv 文件中
    fs.writeFileSync('example_new.csv', csvString, 'utf8');
    return;
    return new Parse.Query("Trigger").limit(200).find();
})
/*
    new Parse.Query("Trigger").limit(200).find().then(list => {
        list.forEach(item => {
            const old = item.get("conditionStr") || ""; // 使用逻辑或运算符来设置默认值为空字符串
            if (old.match(/IODevice/g)) { // 使用全局标志 g 来匹配所有匹配项
                for (let i = 0; i < csvArray.length; i++) {                    
                    var replacedString = old.replace(csvArray[i][eCSV.old],csvArray[i][eCSV.new]);
                    if ((replacedString??"") !== old&&csvArray[i][eCSV.old].length>1){
                        console.log(replacedString);
                        //item.save("conditionStr",replacedString);
                    }
                }
            }
        });
        return ;
    })

new Parse.Query("AutoIO").limit(200).find().then((list) => {
    list.forEach(item => {
        console.log(item);
        const scipt = item.get("ioNames") ?? "422342";
        scipt.forEach(c => {
            //const newC = c.replce( ,"IODevice");
        });
        //const newC = c.replce( ,"IODevice");
    });
    console.log(list.length);
    return new Parse.Query("AutoIO").limit(200).find();
})
*/