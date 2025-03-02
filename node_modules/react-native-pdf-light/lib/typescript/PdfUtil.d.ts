export declare type PageDim = {
    height: number;
    width: number;
};
declare type PdfUtilType = {
    /**
     * Get the number of pages of a pdf.
     */
    getPageCount(source: string): Promise<number>;
    /**
     * Get the dimensions of every page.
     */
    getPageSizes(source: string): Promise<PageDim[]>;
};
/**
 * Utility pdf actions.
 */
export declare const PdfUtil: PdfUtilType;
export {};
