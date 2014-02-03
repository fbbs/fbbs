/**
 * @fileOverview Data adapter for chart and table data before rendering
 * @author pzh
 */

f.dataAdapter = f.Class({
    initialize: f.noop,
    convertBoardData: function (data) {
        var targetList = [];
        f.each(data, function (item) {
            targetList.push({
                url: f.format(f.config.urlFormatter.board, {
                        bid: item.bid
                    }),
                fullName: item.label,
                name: item.board
            });
        });
        return targetList;
    }
});
