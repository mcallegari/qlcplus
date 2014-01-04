// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Full Rows From Center";
        algo.author = "plg";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var center = algo.rgbMapStepCount(width, height) - 1;
            var isEven = (height % 2 == 0);

            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (y == center + step + (isEven ? 1 : 0 ) || y == center - step)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return Math.floor((parseInt(height) + 1) / 2);
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
