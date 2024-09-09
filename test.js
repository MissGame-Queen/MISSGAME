var _URL = "https://ttpunchragic.com/default";
var _LINE_URL = "https://notify-api.line.me/api/notify";
var _LINE_APIKEY = "Bearer DKZ7PLCetuY4jVWt6xhGWIj9fsmTIl2xmHD0xEsqVmu"//大群的LINE APIKEY



/**
 * 回傳資料ID
 * @param {*} path //路徑名稱
 * @returns 
 */
function mygetNodeId(path) {
    //var path = '/forms9/22';//表單:TB16-1103 委外加工單
    var query = db.getAPIQuery(path);
    var nodeId = param.getNewNodeId(query.getKeyFieldId());
    //var nodeId =param.getRootNodeId();
    return nodeId;
}
/**
 * 回傳資料
 * @param {*} path //路徑名稱
 * @returns 
 */
function mygetEntry(path) {
    var query = db.getAPIQuery(path);
    var nodeId = param.getNewNodeId(query.getKeyFieldId());
    var entry = query.getAPIEntry(nodeId);
    return entry;
}
/**
 * 公式重算
 * @param {*} path //路徑名稱
 * @returns 
 */
function myrecalculateAllFormulas(path, id) {
    var query = db.getAPIQuery(path);
    var entry = query.getAPIEntry(id);
    entry.recalculateAllFormulas();
    entry.save();
    response.setMessage('執行完成');
}
/**
 * 會將str內的{}當作key搜尋data然後替換
 * @param {*} str 輸入的字串
 * @param {*} data 輸入的JSON
 * @returns 替換完的字串
 */
function myTemplateLiterals(str, data) {
    return str.replace(/\{(\w+)\}/g, function (match, key) {
        return data[key];
    });
}
/**
 * 排除特定key重複的js陣列
 * @param {JS陣列} interArr 
 * @param {用此KEY取JS內容} filterStr 
 * @returns 回傳interArr[filterStr]不重複的陣列
 */
function myJsonArrFilter(interArr, filterStr) {
    var arr = [];
    if (interArr != null) {
        interArr = interArr.filter(function (item, index, array) {
            var funStr = {};
            funStr = item[filterStr];
            var rtVal = false;
            if (arr.indexOf(funStr) < 0) {
                arr.push(funStr);
                rtVal = true;
            }
            return rtVal;
        });
        return interArr;
    }
    else return arr;
}
/**
 * 排除特定key值不對的js陣列
 * @param {*} interArr 
 * @param {*} filterStr 
 * @param {*} filterValue 
 * @returns 
 */
function myJsonArrFilterValue(interArr, filterStr, filterValue) {
    //var arr =myJsonArrFilter(interArr,filterStr);
    interArr = interArr.filter(function (item, index, array) {
        return item[filterStr] == filterValue ? true : false;
    });
    return interArr;
}
/**
 * 照key值排序JSON
 * @param {*} json 
 * @param {*} key 
 * @returns 
 */
function myJsonArrSort(json, key) {
    for (var j = 1, jl = json.length; j < jl; j++) {
        var temp = json[j],
            val = temp[key],
            i = j - 1;
        while (i >= 0 && json[i][key] > val) {
            json[i + 1] = json[i];
            i = i - 1;
        }
        json[i + 1] = temp;

    }

    return json;
}
/**
 * 
 * @param {*} entry 表單物件
 * @param {
 * key:Value,
 * key:Value,
 * _subtableKey:[{
 * key:value,
 * key:value,
 * }]
 * } js 拋轉的JSON資料
 * @returns 
 */
function myJSToss(entry, js) {
    var Subtable_index = -100;
    for (var key in js) {
        if (typeof js[key] !== "object") {
            entry.setFieldValue(key, js[key]);
        }
        else {
            for (var i = 0; i < js[key].length; i++, Subtable_index--) {
                for (var Subtablekey in js[key][i]) {
                    entry.setSubtableFieldValue(Subtablekey, Subtable_index, js[key][i][Subtablekey]);
                }
            }
        }
    }

    /*子表格的話會導致重複 別開
    entry.loadAllDefaultValues(user);
    entry.loadAllLinkAndLoad();
    entry.recalculateAllFormulas();
    entry.setIfExecuteWorkflow(true);    
    entry.save();
    */
    return entry;
}
/**
 * 時間篩選排序
 * @param {時間陣列} arrTime 
 * @returns 
 */
function myTimeFilterSort(arrTime, arrJs, jsKEY) {
    if (arrJs != null && jsKEY != null) {
        var arrTime = []
        var arrjs = JSON.parse(JSON.stringify(arrJs));
        //取修改時間
        for (var i = 0; i < arrjs.length; i++) {
            for (var key in arrjs[i]) {
                if (key == jsKEY) {
                    arrTime.push((new Date(arrjs[i][key])))
                }
            }
        }
    }
    //移除重複時間
    var arr = [];
    arrTime = arrTime.filter(function (item, index, array) {
        var rtVal = false;
        if (arr.indexOf(item.getTime()) < 0) {
            arr.push(item.getTime());
            rtVal = true;
        }
        return rtVal;
    })
    //排序時間
    arrTime = arrTime.sort(function (a, b) {
        return a > b ? true : false;
    })
    return arrTime;
}
/**
 * 
 * 每日執行方法
 *  
 * */
/**
 * 計算子表格中同型號的累計數量並排版
 * @param {*} inentry 執行的資料入口
 */
function mySum(nodeId, type) {
    var path = '/forms10/22';
    var query = db.getAPIQuery(path);
    var entry;
    var results
    var OutField = 1011854;
    var Output = "";
    var breakSwitch = false;
    var fromVal = 0;
    function queryTOresults() {
        query = db.getAPIQuery(path);
        query.addFilter(1011860, '=', 'No');//是否未過期
        query.setLimitFrom(fromVal);
        results = query.getAPIResultList();
        fromVal += results.length;
    }
    queryTOresults();
    while (results.length > 0) {
        for (var i3 = 0; i3 < results.length; i3++) {
            Output = "";
            switch (type) {
                case 1:
                    entry = results[i3];
                    break;
                default:
                    entry = query.getAPIEntry(nodeId);
                    breakSwitch = true;
            }
            //刷新連結
            entry.loadAllLinkAndLoad();
            //var InputStr = entry.getFieldValue(1010583);
            //依據行數執行
            var jData_Raw = [];
            var ClientArr = [];
            for (var Subtable_i = 0, numLine = -100, SubtableID = 1004406; Subtable_i < entry.getSubtableSize(SubtableID); Subtable_i++, numLine--) {
                //將編號存入陣列   
                jData_Raw[Subtable_i] = {
                    'Number': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1005268),//產品編號
                    'Name': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1004438),//產品名稱
                    'Value': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1004532),//數量
                    'LOT': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1012467) != "" ?
                        entry.getSubtableFieldValue(SubtableID, Subtable_i, 1012467) :
                        entry.getSubtableFieldValue(SubtableID, Subtable_i, 1004404),//訂單單號/委外單號
                    'Unit': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1010244),//單位
                    'Client': entry.getSubtableFieldValue(SubtableID, Subtable_i, 1012474) != "" ?
                        entry.getSubtableFieldValue(SubtableID, Subtable_i, 1012474) :
                        entry.getSubtableFieldValue(SubtableID, Subtable_i, 1004437),//客戶
                };
                ClientArr[Subtable_i] = {
                    'Client': jData_Raw[Subtable_i]['Client'],
                    'Value': parseInt(0)
                };
            }
            var jData = myJsonArrFilter(jData_Raw, "Number");
            ClientArr = myJsonArrFilter(ClientArr, "Client");
            var lineVal = 0;
            for (var ii = 0; ii < ClientArr.length; ii++) {
                for (var i = 0; i < jData.length; i++) {
                    jData[i]['Total'] = parseInt(0);
                    jData[i]['TotalLOT'] = "";
                    //從編號抓名稱並計算總數量
                    var boolswitch = false;
                    for (var j = 0; j < jData_Raw.length; j++) {
                        if (jData[i]['Number'] == jData_Raw[j]["Number"] && jData_Raw[j]["Client"] == ClientArr[ii]['Client']) {
                            jData[i]['Total'] += parseInt(jData_Raw[j]["Value"]);
                            ClientArr[ii]['Value'] += 1;
                            if (jData_Raw[j]['LOT'] != "") {
                                if (jData[i]['TotalLOT'] != "") jData[i]['TotalLOT'] += '、';
                                jData[i]['TotalLOT'] += jData_Raw[j]['LOT'];
                            }
                            boolswitch = true;
                        }
                    }
                    //如果不合條件就不執行下面的
                    if (!boolswitch) continue;
                    lineVal++;
                    //從子表格逐一篩選，如果符合編號將累計數量寫入子表格
                    for (var j = 0, SubtableVal = 1004406; j < entry.getSubtableSize(SubtableVal); j++) {
                        jData[i]['Number'] == entry.getSubtableFieldValue(SubtableVal, j, 1005268) ?
                            entry.setSubtableFieldValue(1010582, entry.getSubtableRootNodeId(SubtableVal, j), jData[i]['Total']) :
                            true;
                    }
                    Output += (lineVal < 9 ? '0' : "") + lineVal + '.' + jData[i]['Client'] + '\n      ' + jData[i]['Number'] + '\n      ' + jData[i]['Name'] + '\n      ' + (jData[i]['TotalLOT'] != "" ? jData[i]['TotalLOT'] + '\n      ' : "") + jData[i]['Total'] + ' ' + jData[i]['Unit'] + '\n\n';
                }
            }
            entry.setFieldValue(1011857, jData.length);//項目數
            entry.setFieldValue(OutField, Output);
            var Str = "";
            for (var j = 0; j < ClientArr.length; j++) {
                if (j != 0) Str += ',';
                Str += ClientArr[j]['Client'] + ':' + ClientArr[j]['Value'];
            }
            entry.setFieldValue(1011858, Str);//客戶清單
            entry.save();
            if (breakSwitch) break;
        }
        if (breakSwitch) break;
        queryTOresults();
    }

    response.setStatus('SUCCESS');
    response.setMessage('執行完成');
}
///**
// 判斷TB08-1005 公告欄是否有需要提醒的佈告欄系統

function myBulletinBoardSystem() {
    //變數設定&抓取今日日期
    var path = '/forms5/108';
    var query = db.getAPIQuery(path);
    var entry = query.getAPIEntry(105);
    var strBulletinBoard = "";
    var strEveryDay = "";
    entry.recalculateFormula(1012171);
    var strToday = entry.getFieldValue(1012171);
    var dateToday = new Date(strToday);
    var jsTB07 = {};
    jsTB07['title'] = "公佈欄系統提醒";
    jsTB07['class'] = "廠長|HR";
    jsTB07['text'] = "";
    //符合條件新增布告欄
    //檢查入職時間
    query = db.getAPIQuery('/tt/3');
    var last3MonthDate = new Date(dateToday);
    last3MonthDate.setMonth(last3MonthDate.getMonth() - 3);
    //篩選3個月內新進人員
    query.addFilter(1000658, '>', last3MonthDate.toISOString().replace(/-/gm, '/').substring(0, 10));
    query.addFilter(1000658, '<=', strToday);
    query.addFilter(1000874, '=', "");
    var results = query.getAPIResultList();

    //依序年資判斷是否符合3、7天、1、2、3月
    for (var i = 0; i < results.length; i++) {
        var interDate = new Date(results[i].getFieldValue(1000658));//到職日
        for (var i2 = 0; i2 < 5; i2++) {
            var checkDate = new Date(interDate);
            switch (i2) {
                case 0:
                    checkDate.setDate(interDate.getDate() + 3);
                    break;
                case 1:
                    checkDate.setDate(interDate.getDate() + 7);
                    break;
                case 2:
                    checkDate.setMonth(interDate.getMonth() + 1);
                    break;
                case 3:
                    checkDate.setMonth(interDate.getMonth() + 2);
                    break;
                case 4:
                    checkDate.setMonth(interDate.getMonth() + 3);
                    break;
            }
            //如果今天符合設定的日期則新增考核表
            if (dateToday.toISOString() == checkDate.toISOString()) {
                query = db.getAPIQuery("/forms5/106");
                var entryAssessment = query.insertAPIEntry();
                entryAssessment.setFieldValue(1010469, results[i].getFieldValue(1000653));
                entryAssessment.setFieldValue(1012613, "系統建立");
                entryAssessment.setFieldValue(1010484, "新人試用期考核");
                for (var Subtable_i = 0, numLine = -100, SubtableID = 1010483; Subtable_i < 9; Subtable_i++, numLine--) {
                    var strSubtable;
                    switch (Subtable_i) {
                        case 0:
                            strSubtable = "責任心";
                            break;
                        case 1:
                            strSubtable = "加班配合度";
                            break;
                        case 2:
                            strSubtable = "出勤狀況";
                            break;
                        case 3:
                            strSubtable = "溝通能力";
                            break;
                        case 4:
                            strSubtable = "Ragic使用情況";
                            break;
                        case 5:
                            strSubtable = "眼力佳";
                            break;
                        case 6:
                            strSubtable = "細心";
                            break;
                        case 7:
                            strSubtable = "機台操作情況";
                            break;
                        case 8:
                            strSubtable = "作業流程及注意事項";
                            break;
                    }
                    entryAssessment.setSubtableFieldValue(1010476, numLine, strSubtable);
                }
                entryAssessment.loadAllLinkAndLoad();
                entryAssessment.save();
                jsTB07['text'] = "[url=" + _URL + "/tt/64" + "/" + entryAssessment.getRootNodeId() + "]員工考核表未結案:" + entryAssessment.getFieldValue(1010469) + entryAssessment.getFieldValue(1010470) + entryAssessment.getFieldValue(1010484) + "[/url]\r\n";
            }
        }
    }




    //每月22號提醒要點斷開連結
    if (strToday.match(/\/\d{2}\/22/) != null) {
        var strCloseList = "";
        var TB05query = db.getAPIQuery('/tt/3');
        strCloseList = "00" + (dateToday.getMonth() + 1);
        strCloseList = strCloseList.substring(strCloseList.length - 2, strCloseList.length);
        TB05query.addFilter(1010867, 'regex', "/" + strCloseList + "/");//篩選同月份
        TB05query.addFilter(1000874, '=', "");//篩選同月份
        var results = TB05query.getAPIResultList();
        strCloseList = "";
        for (var i = 0; i < results.length; i++) {
            if (i != 0) strCloseList += '、';
            strCloseList += results[i].getFieldValue(1000653);
        }
        jsTB07['text'] += "去TB-051202出缺勤記錄表點選月底前點一次\n本月份特休CLOSE名單:" + strCloseList + '\n';
    }
    //3.6.9.12月的25號要季績效考核
    if (strToday.match(/\/(03|06|09|12)\/25/) != null) {
        jsTB07['text'] = jsTB07['text'] + '去TB-051003點選拋轉考核動作按鈕\n';
    }
    var needSave = true;
    if (jsTB07['text'] == "") needSave = false;
    if (needSave) {
        response.setMessage('執行完成');
        query = db.getAPIQuery(path);
        entry = query.insertAPIEntry();
        entry.setFieldValue(1009264, jsTB07['title']);
        entry.setFieldValue(1009260, jsTB07['text']);
        entry.setFieldValue(1009259, jsTB07['personnel']);
        entry.setFieldValue(1012172, jsTB07['class']);
        entry.setFieldValue(1010118, "通泰工業社");
        entry.setFieldValue(1010235, "系統公告");
        entry.loadAllDefaultValues(user);
        entry.save();
    }
    var strPath = '/forms5/106';
    //?員工考核表
    //符合條件新增行事曆
    query = db.getAPIQuery('/forms5/106');
    query.addFilter(1013124, '=', "");//TB08-1012 員工考核表篩選無結案時間
    results = query.getAPIResultList();
    if (results.length > 0) strEveryDay += "員工考核表未結案:\r\n";
    for (var i = 0; i < results.length; i++) {
        strEveryDay += "[url=" + _URL + strPath + "/" + results[i].getRootNodeId() + "]" + results[i].getFieldValue(1010469) + results[i].getFieldValue(1010470) + results[i].getFieldValue(1010484) + "[/url]\r\n";
    }
    strPath = '/ragic-copy/12';
    //?矯正措施總覽表
    //符合條件新增行事曆
    query = db.getAPIQuery('/ragic-copy/12');
    query.addFilter(1006045, 'regex', "[^結案]");//篩選無結案
    results = query.getAPIResultList();
    if (results.length > 0) strEveryDay += "矯正措施總覽表未結案:\r\n";
    for (var i = 0; i < results.length; i++) {
        strEveryDay += "[url=" + _URL + strPath + "/" + results[i].getRootNodeId() + "]請追蹤矯正措施總覽表:" + results[i].getRootNodeId() + "[/url]\r\n";
    }

    //?公告欄
    //將未結案公告輸出到行事曆的提醒事務上
    query = db.getAPIQuery('/forms5/108');
    query.addFilter(1010235, 'regex', "系統公告");//分類用途篩選系統公告未結案公告
    query.addFilter(1010343, 'regex', "No");//篩選未結案公告

    results = query.getAPIResultList();
    strBulletinBoard = "未結案公告欄:\r\n";
    for (var i = 0; i < results.length; i++) {
        strBulletinBoard += "標題:" + results[i].getFieldValue(1009264) + "\r\n";
        strBulletinBoard += "內容:" + results[i].getFieldValue(1009260) + "\r\n\r\n";
    }
    query = db.getAPIQuery('/tt/61');
    query.addFilter(3001245, 'regex', strToday);//篩選該日日期
    results = query.getAPIResultList();
    results[0].setFieldValue(1013123, strBulletinBoard + strEveryDay);
    results[0].setFieldValue(1013892, "Yes");
    results[0].save();


    //response.setStatus('SUCCESS');
    //response.setMessage('執行完成');
}
/**
 * 計算未到貨商品數並顯示到庫存表用
 */
function QuantityNotArrived() {
    var pathMaster = '/forms9/3';
    var query = db.getAPIQuery(pathMaster);
    var arrSubtable_BUY = [];
    var arrSubtable_Arrival = [];
    var arrTB42 = [];
    var arrTB45 = [];
    //從採購單取購買數和進貨數
    query = db.getAPIQuery('/forms9/3');
    query.addFilter(1011001, '=', "");
    //query.addFilter(1011023, 'regex' ,"^(?!尚未進貨).*$");                    
    var TB42entry = query.getAPIResultList();
    for (var i = 0; i < TB42entry.length; i++) {
        TB42entry[i].loadAllLinkAndLoad();
        TB42entry[i].recalculateAllFormulas();
        //紀錄購買數
        for (var Subtable_i = 0, numLine = -100, SubtableID = 1012573; Subtable_i < TB42entry[i].getSubtableSize(SubtableID); Subtable_i++, numLine--) {
            arrSubtable_BUY.push({
                'PO': TB42entry[i].getFieldValue(1002355),//採購單號
                'ProductNumber': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012556),//商品編號
                'Quantity': parseInt(TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012558)),//數量
            });

            query = db.getAPIQuery('/-/5');
            query.addFilter(1002222, '=', TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012556));
            var regex = /^(?!.*-[^物料]).*/;
            query.addFilter(1002230, 'regex', '物料');
            var results = query.getAPIResultList();
            /*
                        if (TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012556) == "800498-370-100") {
                            response.setMessage(TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012556) + '捕獲!');
                            if (results[0].getFieldValue(1002230).match(regex)) {
                                response.setMessage(results[0].getFieldValue(1002230));
                                response.setMessage(results[0].getFieldValue(1002230).match(regex));
                            }
                            return;
                        }
            */
            if (results.length > 0) {
                if (results[0].getFieldValue(1002230).match(regex)) {
                    //HACK 符合條件才抓結構
                    arrTB42.push({
                        'PO': TB42entry[i].getFieldValue(1002355),//採購單號
                        'ProductNumber': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012556),//商品編號
                        'Quantity': parseInt(TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1012558)),//數量
                    });
                }
            }
        }
        //記錄到貨數    
        for (var Subtable_i = 0, numLine = -100, SubtableID = 1002276; Subtable_i < TB42entry[i].getSubtableSize(SubtableID); Subtable_i++, numLine--) {
            arrSubtable_Arrival.push({
                'BS': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1003978),//進貨單號
                'ProductNumber': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1002266),//商品編號
                'Quantity': parseInt(TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1002272)),//數量
            })
        }
        //從通知廠商出貨單取分條後需求數
        for (var Subtable_i = 0, numLine = -100, SubtableID = 1007625; Subtable_i < TB42entry[i].getSubtableSize(SubtableID); Subtable_i++, numLine--) {
            arrTB45.push({
                'SN': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1007610),//通知出貨單號
                'ProductNumber': TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1007612),//商品編號
                'Quantity': parseInt(TB42entry[i].getSubtableFieldValue(SubtableID, Subtable_i, 1007620)),//數量
            })
        }
        TB42entry[i].save();
    }

    //篩選不重複後累計總數
    var arr = [];
    //通知出貨和採購單都進行一次
    for (var i3 = 0; i3 < 2; i3++) {
        var arr2 = (i3 == 0 ? arrTB45 : arrTB42);
        arr = myJsonArrFilter(arr2, 'ProductNumber');
        for (var i = 0; i < arr.length; i++) {
            var Quantity = 0;
            for (var i2 = 0; i2 < arr2.length; i2++) {
                if (arr[i]['ProductNumber'] == arr2[i2]['ProductNumber']) {
                    Quantity += arr2[i2]['Quantity'];
                    //response.setMessage(arr2[i2]['ProductNumber']+'['+arr2[i2]['Quantity']+']');              
                }
            }
            arr[i]['Quantity'] = Quantity;
            //去庫存表找對應編號分析材料並乘上數量
            query = db.getAPIQuery('/-/5');
            query.addFilter(1002222, '=', arr[i]['ProductNumber']);
            query.addFilter(1002230, 'regex', '物料');
            var TB23entry = query.getAPIResultList();
            if (TB23entry.length == 0) continue;
            for (var Subtable_i = 0, numLine = -100, SubtableID = 1005623;
                Subtable_i < TB23entry[0].getSubtableSize(SubtableID);
                Subtable_i++, numLine--) {
                if (i3 == 0) {
                    arrSubtable_BUY.push({
                        'SN': arr[i]['SN'],//通知出貨單號
                        'ProductNumber': TB23entry[0].getSubtableFieldValue(SubtableID, Subtable_i, 1005618),//商品編號
                        'Quantity': parseInt(TB23entry[0].getSubtableFieldValue(SubtableID, Subtable_i, 1005650)) * arr[i]['Quantity'],//數量(1沖幾PCS)
                    })
                }
                else {
                    arrSubtable_BUY.push({
                        'PO': arr[i]['PO'],//通知出貨單號
                        'ProductNumber': TB23entry[0].getSubtableFieldValue(SubtableID, Subtable_i, 1005618),//商品編號
                        'Quantity': parseInt(TB23entry[0].getSubtableFieldValue(SubtableID, Subtable_i, 1005650)) * arr[i]['Quantity'],//數量(1沖幾PCS)
                    })
                }
            }
        }
    }
    //response.setMessage(JSON.stringify(arrSubtable_BUY));
    //篩選不重複編號後統計各編號總購買數
    var arr = myJsonArrFilter(arrSubtable_BUY, 'ProductNumber');
    var arrCONSOLE = [];
    arrCONSOLE.push({
        "進項": arrSubtable_BUY
    });
    arrCONSOLE.push({
        "銷項": arrSubtable_Arrival
    });
    response.setMessage(JSON.stringify(arrCONSOLE));
    for (var i = 0; i < arr.length; i++) {
        var Quantity = 0;
        var valPO = 0;
        var valSN = 0;
        //var test="";
        for (var i2 = 0; i2 < arrSubtable_BUY.length; i2++) {
            if (arr[i]['ProductNumber'] == arrSubtable_BUY[i2]['ProductNumber']) {
                Quantity += parseInt(arrSubtable_BUY[i2]['Quantity']);
                if ("PO" in arrSubtable_BUY[i2]) {
                    valPO += parseInt(arrSubtable_BUY[i2]['Quantity']);
                }
                else if ("SN" in arrSubtable_BUY[i2]) {
                    valSN += parseInt(arrSubtable_BUY[i2]['Quantity']);
                }
                //test+=(arrSubtable_BUY[i2]['PO']!=""?arrSubtable_BUY[i2]['PO']:arrSubtable_BUY[i2]['SN'])+'\n';
            }
        }
        for (var i2 = 0; i2 < arrSubtable_Arrival.length; i2++) {
            if (arr[i]['ProductNumber'] == arrSubtable_Arrival[i2]['ProductNumber']) {
                Quantity -= parseInt(arrSubtable_Arrival[i2]['Quantity']);
            }
        }
        //response.setMessage(arr[i]['ProductNumber'] + '=' + Quantity);
        arr[i]['Quantity'] = Quantity;
        arr[i]['Quantity_PO'] = valPO;
        arr[i]['Quantity_SN'] = valSN;
        //arr[i]['TEST'] = test;

    }
    //response.setMessage(JSON.stringify(arr));

    //如果採購單沒抓到該料號則清空
    query = db.getAPIQuery('/-/5');
    query.addFilter(1012845, 'regex', '.');
    var TB232300entry = query.getAPIResultList();
    for (var i = 0; i < TB232300entry.length; i++) {
        var isChange = false;
        for (var i2 = 0; i2 < arr.length; i2++) {
            if (arr[i2]['ProductNumber'] == TB232300entry[i].getFieldValue(1002222)) {
                isChange = true;
            }
        }
        if (!isChange) {
            TB232300entry[i].setFieldValue(1012510, 0);
            TB232300entry[i].setFieldValue(1012845, "");
            TB232300entry[i].save();
        }
    }

    for (var i = 0; i < arr.length; i++) {
        var strShow = "";
        if (isNaN(arr[i]['Quantity_SN']) || isNaN(arr[i]['Quantity']) || isNaN(arr[i]['Quantity_PO'])) {

            strShow += "資料漏填!";
        }
        else {
            strShow += "已採購:" + (arr[i]['Quantity_PO']) + "\n已通知數量:" + (arr[i]['Quantity_SN']) + "\n已進貨數量:" + (arr[i]['Quantity'] - arr[i]['Quantity_PO']) + "\n已採購未進貨數量:" + arr[i]['Quantity'];
        }
        query = db.getAPIQuery('/-/5');
        query.addFilter(1002222, '=', arr[i]['ProductNumber']);
        TB232300entry = query.getAPIResultList();
        TB232300entry[0].setFieldValue(1012510, arr[i]['Quantity']);
        TB232300entry[0].setFieldValue(1012845, strShow);

        TB232300entry[0].recalculateFormula(1012501);
        TB232300entry[0].recalculateFormula(1005744);
        //TB232300entry[0].setFieldValue(1011012,arr[i]['TEST']  );        
        TB232300entry[0].save();
    }
}
/**每日篩選重算用
 * 
 * @param {String} pathMaster 表單路徑
 * @param {Array} jsFilterData 篩選方式
 * [{"1004491":""}] '尚未有結案日期
 * @param {Array} jsFilterField [可選]，指定重算欄位
 * [1011853] '指定重算連結欄位 
 * @example
 * // 使用例子：
 * FilterRecalculateAllFormulas('path/to/form', {"TEST":123,"TEST2":321}, [1000001,1000002]);
 */
function FilterRecalculateAllFormulas(pathMaster, jsFilterData, jsFilterField) {
    var query = db.getAPIQuery(pathMaster);
    var entry;
    var fromNumberMaster = 0;
    var resultsMaster = [1, 2, 3];
    function queryTOresults() {

        query = db.getAPIQuery(pathMaster);
        var keys = Object.keys(jsFilterData);

        // 遍历键数组
        for (var i = 0; i < keys.length; i++) {
            var key = keys[i];
            var value = jsFilterData[keys[i]];
            query.addFilter(key, 'regex', value);//是否未過期
            //response.setMessage("Key: " + key + ", Value: " + value);
        }
        query.setLimitFrom(fromNumberMaster);
        resultsMaster = query.getAPIResultList();
        fromNumberMaster += resultsMaster.length;
    }
    queryTOresults();
    response.setMessage(resultsMaster.length);
    while (resultsMaster.length > 0) {
        //默認讀取全部資料除非breakSwitch=1
        for (var iResults = 0; iResults < resultsMaster.length; iResults++) {
            entry = resultsMaster[iResults];
            if (typeof jsFilterField === 'undefined' || jsFilterField === null)
                entry.recalculateAllFormulas();
            else {
                for (var i = 0; i < jsFilterField.length; i++) {
                    entry.recalculateFormula(jsFilterField[i]);
                }
            }
            entry.save();
        }
        queryTOresults();
        response.setMessage(resultsMaster.length);
    }
    //response.setStatus('SUCCESS');
    //response.setMessage('執行完成');

}
/**檢查特殊人員資格是否過期
 * 
 * @param {*} nodeId 
 * @param {*} type 
 * @param {*} enterPath 
 */
function myCheckProfessionalQualification(nodeId, type, enterPath) {
    var pathMaster = '/iso/43';
    if (enterPath)
        pathMaster = enterPath;
    var query = db.getAPIQuery(pathMaster);
    var entry;
    var strOut = "";
    var fromNumberMaster = 0;
    var resultsMaster = [1, 2, 3];
    if (type == 9) {
        query = db.getAPIQuery(pathMaster);
        query.setLimitFrom(fromNumberMaster);
        resultsMaster = query.getAPIResultList();
        fromNumberMaster += resultsMaster.length;
    }
    var breakSwitch = false;
    while (resultsMaster.length > 0 && !breakSwitch) {
        //默認讀取全部資料除非breakSwitch=1
        for (var iResults = 0; iResults < resultsMaster.length && !breakSwitch; iResults++) {
            strOut = "";
            switch (type) {
                //全部刷新
                case 9:
                    entry = resultsMaster[iResults];
                    break;
                //單筆刷新
                default:
                    query = db.getAPIQuery(pathMaster);
                    entry = query.getAPIEntry(nodeId);
                    breakSwitch = true;
                    break;
            }
            switch (type) {
                default:
                    //建立JSON
                    var jsonData = {};
                    jsonData["名稱"] = entry.getFieldValue(1013105);
                    jsonData["外訓時效"] = entry.getFieldValue(1013106);
                    jsonData["內訓時效"] = entry.getFieldValue(1013107);
                    jsonData["人員"] = [];
                    //套用內外部計算用欄位ID
                    for (var iFrom = 0; iFrom < 2; iFrom++) {
                        var results;
                        var idMember = "";
                        var path = "";
                        var SubtableID = "";
                        var idName = "";
                        var idTime = "";
                        switch (iFrom) {
                            case 0:
                                path = '/tt/11';
                                idMember = 1013112;
                                SubtableID = 1003464;
                                idName = 1013116;
                                idTime = 1003455;
                                break;
                            case 1:
                                path = '/tt/12';
                                idMember = 1013111;
                                SubtableID = 1003424;
                                idName = 1010139;
                                idTime = 1000701;
                                break;
                        }
                        //篩選出表單資料
                        query = db.getAPIQuery(path);
                        query.addFilter(idMember, 'regex', jsonData["名稱"]);//是否未過期
                        results = query.getAPIResultList();

                        //抓資料
                        for (var i = 0; i < results.length; i++) {
                            for (var Subtable_i = 0; Subtable_i < results[i].getSubtableSize(SubtableID); Subtable_i++) {
                                jsonData["人員"].push({
                                    "姓名": results[i].getSubtableFieldValue(SubtableID, Subtable_i, idName),
                                    "時間": results[i].getFieldValue(idTime).substring(0, 10),
                                    "是否內訓": iFrom,
                                })
                            }
                        }
                    }
                    query = db.getAPIQuery('/tt/3');//人事資料卡
                    query.addFilter(1000874, 'regex', "");//是否未過期
                    results = query.getAPIResultList();
                    //分析資料
                    for (var iFrom = 0; iFrom < 2; iFrom++) {
                        var arr = myJsonArrFilter(jsonData["人員"], "姓名");
                        // response.setMessage(JSON.stringify(jsonData));

                        for (var i = 0; i < arr.length; i++) {
                            var haveOn = false;
                            for (var i3 = 0; i3 < results.length; i3++) {
                                if (arr[i]["姓名"] == results[i3].getFieldValue(1000656)) {
                                    haveOn = true;
                                    break;
                                }
                            }
                            if (!haveOn) {
                                continue;
                            }
                            for (var i2 = 0; i2 < jsonData["人員"].length; i2++) {
                                if (jsonData["人員"][i2]["是否內訓"] == iFrom && arr[i]["姓名"] == jsonData["人員"][i2]["姓名"]) {
                                    if (iFrom) {
                                        if (new Date(arr[i]["最後內訓時間"]) < new Date(jsonData["人員"][i2]["時間"]) || arr[i]["最後內訓時間"] == undefined) {
                                            arr[i]["最後內訓時間"] = jsonData["人員"][i2]["時間"];
                                        }
                                    }
                                    else {
                                        if (new Date(arr[i]["最後外訓時間"]) < new Date(jsonData["人員"][i2]["時間"]) || arr[i]["最後外訓時間"] == undefined) {
                                            arr[i]["最後外訓時間"] = jsonData["人員"][i2]["時間"];
                                        }
                                    }
                                }
                            }
                            if (iFrom == 1) {
                                var interDate = new Date(arr[i]["最後內訓時間"]);
                                var regVal;
                                regVal = jsonData["內訓時效"].match(/(\d+)年/);
                                if (regVal !== null)
                                    interDate.setFullYear(interDate.getFullYear() + parseInt(regVal[1]));

                                regVal = jsonData["內訓時效"].match(/(\d+)月/);
                                if (regVal !== null)
                                    interDate.setMonth(interDate.getMonth() + parseInt(regVal[1]));

                                regVal = jsonData["內訓時效"].match(/(\d+)日/);
                                if (regVal !== null)
                                    interDate.setDate(interDate.getDate() + parseInt(regVal[1]));

                                arr[i]["內訓有效期限"] = interDate.getFullYear() + '/' +
                                    (interDate.getMonth() + 1 < 10 ? '0' : '') + (interDate.getMonth() + 1) + '/' +
                                    (interDate.getDate() < 10 ? '0' : '') + interDate.getDate();
                                if (jsonData["內訓最快到期日"] == undefined || new Date(jsonData["內訓最快到期日"]) > new Date(arr[i]["內訓有效期限"]))
                                    jsonData["內訓最快到期日"] = arr[i]["內訓有效期限"];
                                strOut += arr[i]["姓名"] +
                                    "，外訓有效時間 " + (arr[i]["最後外訓時間"] == undefined ? "N/A" : (arr[i]["最後外訓時間"] + ' ~ ' + (arr[i]["最後外訓時間"] == arr[i]["外訓有效期限"] ? '∞' : arr[i]["外訓有效期限"]))) +
                                    "，內訓有效時間 " + (arr[i]["最後內訓時間"] == undefined ? "N/A" : (arr[i]["最後內訓時間"] + ' ~ ' + (arr[i]["最後內訓時間"] == arr[i]["內訓有效期限"] ? '∞' : arr[i]["內訓有效期限"]))) +
                                    '\r\n';
                            }
                            else {
                                var interDate = new Date(arr[i]["最後外訓時間"]);
                                var regVal;
                                regVal = jsonData["外訓時效"].match(/(\d+)年/);
                                if (regVal !== null)
                                    interDate.setFullYear(interDate.getFullYear() + parseInt(regVal[1]));

                                regVal = jsonData["外訓時效"].match(/(\d+)月/);
                                if (regVal !== null)
                                    interDate.setMonth(interDate.getMonth() + parseInt(regVal[1]));

                                regVal = jsonData["外訓時效"].match(/(\d+)日/);
                                if (regVal !== null)
                                    interDate.setDate(interDate.getDate() + parseInt(regVal[1]));

                                arr[i]["外訓有效期限"] = interDate.getFullYear() + '/' +
                                    (interDate.getMonth() + 1 < 10 ? '0' : '') + (interDate.getMonth() + 1) + '/' +
                                    (interDate.getDate() < 10 ? '0' : '') + interDate.getDate();
                                if (jsonData["外訓最快到期日"] == undefined || new Date(jsonData["外訓最快到期日"]) > new Date(arr[i]["外訓有效期限"]))
                                    jsonData["外訓最快到期日"] = arr[i]["外訓有效期限"];
                            }

                        }
                    }
                    var lastDate = "";
                    if (jsonData["外訓最快到期日"] && jsonData["內訓最快到期日"]) {
                        var date1 = new Date(jsonData["外訓最快到期日"]);
                        var date2 = new Date(jsonData["內訓最快到期日"]);
                        if (date1.toString() != "Invalid Date" && date2.toString() != "Invalid Date") {
                            if (date1 < date2) {
                                if (jsonData["外訓時效"] != "")
                                    lastDate = jsonData["外訓最快到期日"];
                            } else {
                                if (jsonData["內訓時效"] != "")
                                    lastDate = jsonData["內訓最快到期日"];
                            }
                        }
                        else if (date1.toString() != "Invalid Date" && jsonData["外訓時效"] != "")
                            lastDate = jsonData["外訓最快到期日"];
                        else if (date2.toString() != "Invalid Date" && jsonData["內訓時效"] != "")
                            lastDate = jsonData["內訓最快到期日"];
                        else {
                            // 处理日期无效的情况，例如日期为空或格式不正确
                            response.setMessage("日期无效");
                        }
                    }
                    entry.setFieldValue(1013117, lastDate);
                    entry.setFieldValue(1013114, strOut);

                    entry.save();
            }
        }
    }
    response.setStatus('SUCCESS');
    response.setMessage('執行完成');
}

function POST_LINE(config) {
    if (config["Url"].length === 0) {
        response.setMessage("請填入[Url]參數");
    } else if (config["Msg"].length === 0) {
        response.setMessage("請填入[Msg]參數");
    } else {
        util.setHeader("Authorization", config["Authorization"]);
        var str = "";
        str = util.postURL(config["Url"], config["Msg"]);
        //response.setMessage(str);
    }
}
/*
樣板
function myTemplate(nodeId, type,enterPath) {
    var pathMaster = '/iso/30';
    if (enterPath)
        pathMaster = enterPath;
    var query = db.getAPIQuery(pathMaster);
    var entry;
    var strOut = "";
    var fromNumberMaster = 0;
    var resultsMaster  = [1, 2, 3];
    function queryTOresults() {
        if (type == 9) {
            query = db.getAPIQuery(pathMaster);
            query.addFilter(1005701, 'regex', regexStr);//是否未過期
            query.setLimitFrom(fromNumberMaster);

            resultsMaster  = query.getAPIResultList();
            fromNumberMaster += resultsMaster .length;
        }
    }
    queryTOresults();


    var breakSwitch = false;
    while (resultsMaster .length > 0 && !breakSwitch) {
        //默認讀取全部資料除非breakSwitch=1
        for (var iResults = 0; iResults < resultsMaster .length && !breakSwitch; iResults++) {
            strOut = "";
            switch (type) {
                //全部刷新
                case 9:
                    entry = resultsMaster [iResults];
                    break;
                //單筆刷新
                default:
                    query = db.getAPIQuery(pathMaster);
                    entry = query.getAPIEntry(nodeId);
                    breakSwitch = true;
                    break;
            }
            switch (type) {

                default:
                    querySubtable.insertAPIEntry();
                    var MOarr = [];
                    entry.deleteSubtableRowAll();
                        for (var varBox_i = 0; varBox_i < Formulas.length; varBox_i++) {
                            //偶爾會跳錯誤，過一下在執行
                            var query1 = db.getAPIQuery('/forms3/30');

                            regexStr = Formulas[varBox_i];
                            //regexStr = regexStr.match(/\[(.*)\]/)[1];                                                        
                            query1.addFilter(1000495, '=', regexStr);
                            results = query1.getAPIResultList();
                            response.setMessage(results.length);

                            for (var j = 0, SubtableVal = 1013247, numLine = iSubtable * -1 - 100; j < results.length; j++) {
                                response.setMessage(results[j].getRootNodeId());
                                var indexNum = iSubtable + j < entry.getSubtableSize(SubtableVal) ? entry.getSubtableRootNodeId(SubtableVal, iSubtable + j) : numLine - j;
                                entry.setSubtableFieldValue(1013248, indexNum, results[j].getRootNodeId());
                                entry.setSubtableFieldValue(1013256, indexNum, results[j].getFieldValue(1008435));
                                entry.setSubtableFieldValue(1013243, indexNum,
                                    "[url=" + _URL + path + '/' + results[j].getRootNodeId() + ']' + results[j].getFieldValue(1000495) + "[/url]");
                                entry.setSubtableFieldValue(1013244, indexNum, results[j].getFieldValue(1004407));//原本MO
                                entry.setSubtableFieldValue(1013245, indexNum, entry.getFieldValue(1013230));//移到MO
                            }
                            iSubtable += results.length;
                        }

                        for (var j = iSubtable, SubtableVal = 1013247, numLine = -100; j < entry.getSubtableSize(SubtableVal); j++, numLine--) {
                            entry.deleteSubtableRowByRowNumber(SubtableVal, j)
                        }
                    var _yyyyMM = new Date(entry.getFieldValue(1006638));
                    var jsMaster = {
                        'Date': _yyyyMM.getFullYear() + '/' + (parseInt(_yyyyMM.getMonth() + 1) >= 10 ? '' : '0') + parseInt(_yyyyMM.getMonth() + 1),//日期
                    }
                    function checkLink(path, DateField, LinkField) {

                        query = db.getAPIQuery(path);
                        query.addFilter(DateField, 'regex', jsMaster['Date']);
                        var results = query.getAPIResultList();
                        var fromNumber = 0;
                        while (results.length > 0) {
                            for (var i in results) {
                                if (results[i].getFieldValue(LinkField) == "") {
                                    var _yyyyMM = new Date(results[i].getFieldValue(DateField));
                                    results[i].setFieldValue(LinkField, _yyyyMM.getFullYear() + '/' + (parseInt(_yyyyMM.getMonth() + 1) >= 10 ? '' : '0') + parseInt(_yyyyMM.getMonth() + 1));
                                    results[i].save();
                                }
                            }
                            fromNumber += results.length;
                            query = db.getAPIQuery(path);
                            query.setLimitFrom(fromNumber);
                            query.addFilter(DateField, 'regex', jsMaster['Date']);
                            results = query.getAPIResultList();
                        }
                        return fromNumber;
                        strTest += '成品檢驗紀錄表:' + fromNumber + '\n';

                    }
                    entry.setFieldValue(1010525, 'Yes');
                    entry.setFieldValue(1012120, strTest);
                    TB24entry.loadAllDefaultValues(user);
                    TB24entry.loadAllLinkAndLoad();
                    TB24entry.recalculateAllFormulas();
                    TB24entry.setIfExecuteWorkflow(true);
                    TB24entry.save();
                    TB24entry=query.getAPIEntry(TB24entry.getRootNodeId());
                    TB24entry.loadAllDefaultValues(user);
                    TB24entry.loadAllLinkAndLoad();
                    TB24entry.recalculateAllFormulas();
                    TB24entry.recalculateFormula(1020004)
                    TB24entry.setIfExecuteWorkflow(true);
                    entry.setFieldValue(1010525, 'No');
                    entry.save();
                    switch (type) {
                        case 0:
                            response.setStatus('SUCCESS');
                            response.setMessage('執行完成');
                            response.setOpenURL()//跳轉到該筆資料
                            return;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
        queryTOresults();
    }
    response.setStatus('SUCCESS');
    response.setMessage('執行完成');
}
*/
