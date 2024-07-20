try {
    data = JSON.parse(data);
    let arrReturn = [];
    if (data?.data?.tagList?.length > 0) {
        data.data.tagList.forEach(item => {
            let jsonData = {
                "ant": item.ant,
                "epc": item.epc
            };
            arrReturn.push(jsonData);
        });
        return JSON.stringify(arrReturn);
    }
} catch (e) {
}
return "";