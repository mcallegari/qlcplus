// Development tool access
var testAlgo;

(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Fill Unfill Columns From Center";
        algo.author = "plg";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var center = Math.floor((parseInt(width) + 1) / 2) - 1;
            var isEven = (width % 2 == 0);
            var centerStep = center + 1;

            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (step < centerStep)
                    {
                        if (x <= center + step + (isEven ? 1 : 0 ) && x >= center - step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        var step2 = step - centerStep
                        if (x <= center + step2 + (isEven ? 1 : 0 ) && x >= center - step2)
                            map[y][x] = 0;
                        else
                            map[y][x] = rgb;
                    }
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return Math.floor((parseInt(width) + 1) / 2) * 2 - 1;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
