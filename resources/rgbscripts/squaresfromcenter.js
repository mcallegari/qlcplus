// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 2;
        algo.name = "Squares From Center";
        algo.author = "plg";
        algo.acceptColors = 2;
        algo.properties = new Array();
        algo.fillSquares = 0;
        algo.properties.push("name:fillSquares|type:list|display:Fill squares|values:No,Yes|write:setFill|read:getFill");
        
        algo.setFill = function(_fill)
        {
            if (_fill == "Yes")
                algo.fillSquares = 1;
            else
                algo.fillSquares = 0;
        }

        algo.getFill = function()
        {
            if (algo.fillSquares == 1)
                return "Yes";
            else
                return "No";
        }

        algo.rgbMap = function(width, height, rgb, step)
        {
            var widthCenter = Math.floor((parseInt(width) + 1) / 2) - 1;
            var heightCenter = Math.floor((parseInt(height) + 1) / 2) - 1;
            var isWidthEven = (width % 2 == 0);
            var isHeightEven = (height % 2 == 0);

            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (algo.fillSquares == 1)
                    {
                        if ((x <= widthCenter + step + (isWidthEven ? 1 : 0 ) && x >= widthCenter - step) &&
                            (y <= heightCenter + step + (isHeightEven ? 1 : 0) && y >= heightCenter - step))
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                    }
                    else
                    {
                        if ((x == widthCenter + step + (isWidthEven ? 1 : 0 ) || x == widthCenter - step) &&
                                (y <= heightCenter + step + (isHeightEven ? 1 : 0) && y >= heightCenter - step))
                            map[y][x] = rgb;
                        else if ((y == heightCenter + step + (isHeightEven ? 1 : 0 ) || y == heightCenter - step) &&
                                (x <= widthCenter + step + (isWidthEven ? 1 : 0) && x >= widthCenter - step))
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            width = parseInt(width);
            height = parseInt(height);
            var max = width > height ? width : height;
            return Math.floor((max + 1) / 2);
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
